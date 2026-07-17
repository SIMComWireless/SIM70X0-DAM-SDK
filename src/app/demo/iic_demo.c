/**
 * @file iic_demo.c
 * @brief I2C (IIC) master demo utilities.
 *
 * Implements initialization and simple master read/write helpers used by
 * demo code. The module uses the QAPI I2C master APIs and provides a
 * synchronous wrapper using a semaphore for completion notification.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "IIC_demo"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "txm_module.h"
#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_txm_base.h"
#include "qapi/qapi_timer.h"
#include "qapi/qapi_i2c_master.h"

#include "iic_demo.h"
#include "../Easylogger/elog.h"

static int completion = 0;
static int32_t CB_Parameter = 0;
static void	 *iic_client_handle = NULL;
static TX_SEMAPHORE *iic_asyncHandle;     //for IIC read\write sync
static TX_MUTEX 	 *iic_protection;       //for IIC multi thread operation
static qapi_I2CM_Config_t config;         //global configuration for IIC

extern TX_BYTE_POOL *byte_pool_dam;

/**
  * @brief  IIC init
  * @param 
  * @note   
  * @retval 
  */
DAM_Status_t iic_init(void)
{
  qapi_Status_t res = QAPI_OK;
  UINT status;
  static uint8_t iic_initiled = 0;

  if(iic_initiled)
      return DAM_STATUS_SUCCESS;

  status = txm_module_object_allocate(&iic_asyncHandle, sizeof(TX_SEMAPHORE));
	if(status != TX_SUCCESS)
  {
    log_e("Failed to allocate iic_asyncHandle: %d", status);
  }
	status = tx_semaphore_create(iic_asyncHandle,"iic_asyncHandle", 0);
	if(status != TX_SUCCESS)
  {
    log_e("Failed to create iic_asyncHandle: %d", status);
  }

  status = txm_module_object_allocate(&iic_protection, sizeof(TX_MUTEX));
	if(status != TX_SUCCESS)
  {
    log_e("Failed to allocate iic_protection: %d", status);
  }
	status = tx_mutex_create(iic_protection,"iic_protection", TX_NO_INHERIT);
	if(status != TX_SUCCESS)
  {
    log_e("Failed to create iic_protection: %d", status);
  }

	res = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_001_E, &iic_client_handle);
	if(res != QAPI_OK) {
		log_e("qapi_I2CM_Open failed !!! %d", res);
		return DAM_STATUS_ERROR;
	}

   config.bus_Frequency_KHz = 100; 
   config.SMBUS_Mode        = 0;
   config.slave_Max_Clock_Stretch_Us = 0;
   config.core_Configuration1 = 0;
   config.core_Configuration2 = 0;
	
	qapi_Timer_Sleep(100, QAPI_TIMER_UNIT_MSEC , true);

  iic_initiled = 1;

  log_i("iic initiled");

	return DAM_STATUS_SUCCESS;
}

uint32_t iic_status;
/**
  * @brief  IIC client callback when data transfering complete.
  * @param 
  * @note   
  * @retval 
  */
void client_callback (const uint32_t status, void *cb_para)
{ 
    tx_semaphore_put(iic_asyncHandle);
}

/**
 * @brief I2C master write helper.
 *
 * Sends register/address bytes followed by payload to the specified I2C
 * slave device using the QAPI I2C master transfer API.
 *
 * @param[in] slave_Address 7-bit slave address.
 * @param[in] reg Pointer to register/address bytes to send before payload.
 * @param[in] reg_len Number of register/address bytes.
 * @param[in] write_buffer Pointer to payload data to write.
 * @param[in] len Length of payload in bytes.
 *
 * @return qapi_Status_t QAPI_OK on success, otherwise an error code.
 */
qapi_Status_t i2c_write (uint8_t slave_Address, uint8_t *reg, uint8_t reg_len,uint8_t *write_buffer, uint16_t len)
{
   qapi_Status_t res = QAPI_OK;

   qapi_I2CM_Descriptor_t desc[2];
   uint16_t  i;
   uint32_t  transferred[2] = {0};
   uint8_t *temp_buf;

   if((reg_len == 0) || (len == 0))
    return QAPI_ERROR;

  // Configure the slave address
   config.slave_Address = slave_Address;

  tx_byte_allocate(byte_pool_dam, (VOID **) &temp_buf, reg_len+len, TX_NO_WAIT);
  memset(temp_buf, 0, reg_len+len);
  memcpy(temp_buf,reg,reg_len);
  memcpy(&temp_buf[reg_len],write_buffer,len);

   desc[0].buffer      = temp_buf;
   desc[0].length      = reg_len+len;
   desc[0].transferred = (uint32_t)&transferred[0];
   desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;


   tx_mutex_get(iic_protection, TX_WAIT_FOREVER);
   res = qapi_I2CM_Transfer (iic_client_handle, &config, &desc[0], 1, client_callback, &CB_Parameter, 0);
   tx_mutex_put(iic_protection);
    if (QAPI_OK == res)
    {
      tx_semaphore_get(iic_asyncHandle,TX_WAIT_FOREVER);
    }
    tx_byte_release(temp_buf);

   return res;
}

/**
 * @brief I2C master read helper.
 *
 * Performs a combined write-then-read transfer: the `reg` bytes are
 * written first (typically the register address), then `len` bytes are
 * read into `read_buffer`.
 *
 * @param[in] slave_Address 7-bit slave address.
 * @param[in] reg Pointer to register/address bytes to send before reading.
 * @param[in] reg_len Number of register/address bytes.
 * @param[out] read_buffer Buffer to receive read data.
 * @param[in] len Number of bytes to read.
 *
 * @return qapi_Status_t QAPI_OK on success, otherwise an error code.
 */
qapi_Status_t i2c_read(uint8_t slave_Address, uint8_t *reg, uint8_t reg_len, uint8_t *read_buffer, uint16_t len)
{
	qapi_Status_t res = QAPI_OK;
	qapi_I2CM_Descriptor_t desc[2];
	uint32_t  transferred[2] = {0};
	int i = 0;

  if((reg_len == 0) || (len == 0))
    return QAPI_ERROR;
	// Configure the slave address
	config.slave_Address     = slave_Address;
	
  desc[0].buffer      = reg;
  desc[0].length      = reg_len;
  desc[0].transferred = (uint32_t)&transferred[0];
  desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;
  
  desc[1].buffer      = read_buffer;
  desc[1].length      = len;
  desc[1].transferred = (uint32_t)&transferred[1];
  desc[1].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ | QAPI_I2C_FLAG_STOP;

  tx_mutex_get(iic_protection, TX_WAIT_FOREVER);
	res = qapi_I2CM_Transfer (iic_client_handle, &config, &desc[0], 2, client_callback, &CB_Parameter, 0);
  tx_mutex_put(iic_protection);
	if (QAPI_OK == res){
    tx_semaphore_get(iic_asyncHandle,TX_WAIT_FOREVER);
	}
	
	return res;
}