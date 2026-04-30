/**
 * @file iic_demo.h
 * @brief Public declarations for the I2C demo helpers.
 *
 * Declares the initialization and read/write helper prototypes provided by
 * iic_demo.c for demo purposes.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __IIC_DEMO_H__
#define __IIC_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/

DAM_Status_t iic_init(void);
qapi_Status_t i2c_write (uint8_t slave_Address, uint8_t *reg, uint8_t reg_len, uint8_t *write_buffer, uint16_t len);
qapi_Status_t i2c_read(uint8_t slave_Address, uint8_t *reg, uint8_t reg_len, uint8_t *read_buffer, uint16_t len);

#endif

