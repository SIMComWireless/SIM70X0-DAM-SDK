/**
 * @file GPIO_demo.c
 * @brief GPIO demo helpers.
 *
 * GPIO initialization, read/write and interrupt helper functions used by
 * the demo applications.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "Gpio_demo"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <locale.h>
#include "qapi/qapi_txm_base.h"
#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_timer.h"
#include "qapi/qapi_gpioint.h"
#include "qapi/qapi_tlmm.h"

#include "GPIO_demo.h"
#include "../Easylogger/elog.h"

#define GPIONUMMAX	0x16
uint16_t GpioCtr[GPIONUMMAX*2];

/**
 * @brief GPIO interrupt callback.
 *
 * Invoked by the GPIO interrupt subsystem when an interrupt occurs.
 *
 * @param[in] data Pointer to the callback data supplied by the kernel.
 */
static void GPIO_interrupt_call_back(qapi_GPIOINT_Callback_Data_t *data)
{
  log_i("Enter GPIO_interrupt_call_back");
}


 qapi_Instance_Handle_t gpio_handle_gpio;

/**
  * @brief  register a GPIO interrupt
  * @param  
  * @note   
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
DAM_Status_t gpio_interrupt_register(uint32_t gpio_num)
{

  qapi_Status_t  nStatus;
  qapi_GPIOINT_CB_t pfnCallback = GPIO_interrupt_call_back;
  qapi_GPIOINT_Trigger_e trigger_level;
  qapi_GPIOINT_Priority_e priority;
  
  trigger_level = QAPI_GPIOINT_TRIGGER_EDGE_FALLING_E;
  priority = QAPI_GPIOINT_PRIO_HIGHEST_E;
  /* Pass NULL as user_data (no user context required) */
  nStatus = qapi_GPIOINT_Register_Interrupt(&gpio_handle_gpio,         // Pass in a pointer to the handle
                                            gpio_num,                  // GPIO number
                                            pfnCallback,               // Callback fn pointer
                                            NULL,                      // user callback data
                                            trigger_level,             // Level trigger
                                            priority,                  // Priority of interrupt
                                            false );                   // Maskable Interrupt

  if ( QAPI_OK == nStatus )
  {
    qapi_GPIOINT_Enable_Interrupt(&gpio_handle_gpio,gpio_num);
    return DAM_STATUS_SUCCESS;
  } 

  return DAM_STATUS_ERROR;
}

/**
  * @brief  Deregister a GPIO interrupt
  * @param  
  * @note   
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
DAM_Status_t gpio_interrupt_deregister(uint32_t gpio_num)
{
   qapi_Status_t  nStatus;

   qapi_GPIOINT_Disable_Interrupt(&gpio_handle_gpio,gpio_num);

   nStatus = qapi_GPIOINT_Deregister_Interrupt(&gpio_handle_gpio , gpio_num);

  if ( QAPI_OK == nStatus )
  {
    log_i("Deregister INT well");
    return DAM_STATUS_SUCCESS;
  } 

  return DAM_STATUS_ERROR;
}


/**
  * @brief  GPIO init.
  * @param  gpio_num --pin number, refer to DAM_GPIO_PINMAP
  * @param  dir --GPIO_INPUT_DIR or GPIO_OUTPUT_DIR
  * @param  level --GPIO_LOW_LEVEL or GPIO_HIGH_LEVEL
  * @note   
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
/**
 * @brief Configure a GPIO pin direction and initial level.
 *
 * Requests a TLMM GPIO ID for the specified pin, configures it and
 * stores the mapping in the local `GpioCtr` table for later operations.
 *
 * @param[in] gpio_num Pin number (use values from `DAM_GPIO_PINMAP`).
 * @param[in] dir Direction: `GPIO_INPUT_DIR` or `GPIO_OUTPUT_DIR`.
 * @param[in] level Initial level when configured as output.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gpio_init(uint32_t gpio_num, GPIO_Dir_t dir , GPIO_Level_t level)
{
      qapi_Status_t  status = QAPI_OK;
      qapi_TLMM_Config_t  tlmm_config;
      qapi_GPIO_ID_t     v_gpio_id;    
      qapi_GPIO_Value_t  gpio_value;
      uint16_t i, n;

      tlmm_config.pin = gpio_num;
      tlmm_config.func = 0;
      tlmm_config.pull = QAPI_GPIO_PULL_UP_E;
      tlmm_config.drive = QAPI_GPIO_2MA_E; 

      if(dir == GPIO_INPUT_DIR)
        tlmm_config.dir = QAPI_GPIO_INPUT_E;
      else
        tlmm_config.dir = QAPI_GPIO_OUTPUT_E;

      n = (uint32_t) gpio_num;

      /* Release any previously assigned ID for this pin. */
      for(i = 0; i<GPIONUMMAX; i++)
      {
        if(GpioCtr[i*2] == n && GpioCtr[i*2 +1] != 0 )
        {
          v_gpio_id = GpioCtr[i*2 +1];
          status = qapi_TLMM_Release_Gpio_ID(&tlmm_config, v_gpio_id);
          log_i("Release status: %d , Pin: %d , Id:%d", status,  n, v_gpio_id);
          GpioCtr[i*2] = 0;
          GpioCtr[i*2 +1] = 0;
          break;
        }
      }

      status = qapi_TLMM_Get_Gpio_ID( &tlmm_config, &v_gpio_id);
      if (status != QAPI_OK)
      {
          return DAM_STATUS_ERROR;
      } 

      status = qapi_TLMM_Config_Gpio(v_gpio_id, &tlmm_config);
      if (status != QAPI_OK)
      {
          return DAM_STATUS_ERROR;
      }

      for(i = 0; i<GPIONUMMAX; i++)
      {
        if(GpioCtr[i*2] == 0)
        {
          GpioCtr[i*2] = (uint32_t)gpio_num;
          GpioCtr[i*2+1] = v_gpio_id;
          break;
        }
      }

      if(dir == GPIO_OUTPUT_DIR)
      {
        gpio_value = (qapi_GPIO_Value_t)level;
        status = qapi_TLMM_Drive_Gpio(v_gpio_id, (uint32_t)gpio_num, gpio_value);
        if(status != QAPI_OK) 
        {
          return DAM_STATUS_ERROR;
        } 
      }
      return DAM_STATUS_SUCCESS;
}

/**
  * @brief  write GPIO pin
  * @param  gpio_num --pin number, refer to DAM_GPIO_PINMAP
  * @param  level --GPIO_LOW_LEVEL or GPIO_HIGH_LEVEL
  * @note   
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
DAM_Status_t gpio_write_pin(uint32_t gpio_num, GPIO_Level_t level)
{
    	qapi_Status_t      status;
	    qapi_GPIO_ID_t     v_gpio_id;    
	    qapi_GPIO_Value_t  gpio_value;

	    uint16_t n, i;
	    gpio_value = (qapi_GPIO_Value_t)level;
	    n = (uint32_t) gpio_num;

      for(i = 0; i<GPIONUMMAX; i++)
      {
        if(GpioCtr[i*2] == n && GpioCtr[i*2 +1] != 0 )
        {
          v_gpio_id = GpioCtr[i*2 +1];
          status = qapi_TLMM_Drive_Gpio(v_gpio_id, (uint32_t)gpio_num, gpio_value);
          break;
        }
      }

    return DAM_STATUS_SUCCESS;
}

/**
  * @brief  read GPIO pin 
  * @param  gpio_num --pin number, refer to DAM_GPIO_PINMAP
  * @param  level -- pointer to read level
  * @note   
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
/**
 * @brief Read the current level of a GPIO pin.
 *
 * Looks up the pin in the internal GpioCtr table and reads its
 * current value via qapi_TLMM_Read_Gpio().
 *
 * @param[in]  gpio_num  Pin number (see DAM_GPIO_PINMAP).
 * @param[out] level     Pointer that receives the pin level.
 *
 * @retval DAM_STATUS_SUCCESS  Pin read successfully.
 * @retval DAM_STATUS_ERROR    Pin not registered or read failed.
 */
DAM_Status_t gpio_read_pin(uint32_t gpio_num, GPIO_Level_t *level)
{
    qapi_Status_t      status = QAPI_ERROR;
    qapi_GPIO_ID_t     v_gpio_id = 0;
    qapi_GPIO_Value_t  gpio_value = QAPI_GPIO_LOW_VALUE_E;
    uint32_t n, i;
    bool found = false;

    n = (uint32_t)gpio_num;

    for (i = 0; i < GPIONUMMAX; i++)
    {
        if (GpioCtr[i * 2] == n && GpioCtr[i * 2 + 1] != 0)
        {
            v_gpio_id = GpioCtr[i * 2 + 1];
            status = qapi_TLMM_Read_Gpio(v_gpio_id, gpio_num, &gpio_value);
            found = true;
            break;
        }
    }

    if (!found)
    {
        log_e("gpio_read_pin: pin %d not registered", gpio_num);
        return DAM_STATUS_ERROR;
    }

    if (status != QAPI_OK)
    {
        log_e("gpio_read_pin: read failed for pin %d", gpio_num);
        return DAM_STATUS_ERROR;
    }

    *level = (gpio_value == QAPI_GPIO_LOW_VALUE_E) ? GPIO_LOW_LEVEL
                                                    : GPIO_HIGH_LEVEL;
    return DAM_STATUS_SUCCESS;
}
