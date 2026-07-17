/**
 * @file iic_demo.h
 * @brief I2C (IIC) master demo helper API.
 *
 * Initialisation and synchronous read/write helpers for the I2C master
 * peripheral used by the demo applications.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __IIC_DEMO_H__
#define __IIC_DEMO_H__

#include "../application/ThreadX.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the I2C master peripheral and internal synchronisation.
 *
 * Safe to call multiple times — subsequent calls are no-ops.
 *
 * @retval DAM_STATUS_SUCCESS  Initialised (or already initialised).
 * @retval DAM_STATUS_ERROR    I2C open failed.
 */
DAM_Status_t iic_init(void);

/**
 * @brief Write data to an I2C slave device.
 *
 * Sends @p reg_len bytes from @p reg followed by @p len bytes from
 * @p write_buffer to the slave at @p slave_Address.
 *
 * @param[in] slave_Address  7-bit I2C slave address.
 * @param[in] reg            Register / address bytes to send first.
 * @param[in] reg_len        Number of bytes in @p reg.
 * @param[in] write_buffer   Payload data to write.
 * @param[in] len            Number of bytes in @p write_buffer.
 *
 * @retval QAPI_OK   Transfer completed successfully.
 * @retval Other     QAPI error code.
 */
qapi_Status_t i2c_write(uint8_t slave_Address, uint8_t *reg,
                         uint8_t reg_len, uint8_t *write_buffer,
                         uint16_t len);

/**
 * @brief Read data from an I2C slave device.
 *
 * Writes @p reg_len bytes from @p reg (typically the register address)
 * then reads @p len bytes into @p read_buffer.
 *
 * @param[in]  slave_Address  7-bit I2C slave address.
 * @param[in]  reg            Register / address bytes to write first.
 * @param[in]  reg_len        Number of bytes in @p reg.
 * @param[out] read_buffer    Buffer to receive the read data.
 * @param[in]  len            Number of bytes to read.
 *
 * @retval QAPI_OK   Transfer completed successfully.
 * @retval Other     QAPI error code.
 */
qapi_Status_t i2c_read(uint8_t slave_Address, uint8_t *reg,
                        uint8_t reg_len, uint8_t *read_buffer,
                        uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __IIC_DEMO_H__ */
