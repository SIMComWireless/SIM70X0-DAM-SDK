/**
 * @file ThreadX.h
 * @brief Demo thread prototypes and common status type.
 *
 * Declares the DAM_Status_t enumeration and all demo thread entry
 * function prototypes used by the application layer.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __THREADX_H__
#define __THREADX_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic status codes returned by DAM helper functions.
 */
typedef enum
{
    DAM_STATUS_SUCCESS, /**< Operation completed successfully. */
    DAM_STATUS_ERROR,   /**< Operation failed. */
} DAM_Status_t;

/**
 * @name Thread Entry Prototypes
 * @{
 */

/** @brief Hello-world demo thread.        @param Param Unused. */
void Hello_world_Thread(void *Param);

/** @brief Socket (TCP/UDP) demo thread.   @param Param Unused. */
void Socket_Thread(void *Param);

/** @brief GPIO demo thread.               @param Param Unused. */
void GPIO_Thread(void *Param);

/** @brief MQTT client demo thread.        @param Param Unused. */
void MQTT_Thread(void *Param);

/** @brief AT command demo thread.         @param Param Unused. */
void AT_Thread(void *Param);

/** @brief ADC demo thread.                @param Param Unused. */
void ADC_Thread(void *Param);

/** @brief File-system demo thread.        @param Param Unused. */
void FS_Thread(void *Param);

/** @brief HTTP client demo thread.        @param Param Unused. */
void HTTP_Thread(void *Param);

/** @brief GPS/GNSS demo thread.           @param Param Unused. */
void GPS_Thread(void *Param);

/** @brief OTA update download thread.     @param Param Unused. */
void UPDATE_Thread(void *Param);

/** @brief OTA update state-check thread.  @param Param Unused. */
void UPDATE_State_Check_Thread(void *Param);

/** @brief PWM demo thread.                @param Param Unused. */
void PWM_Thread(void *Param);

/** @brief I2C (IIC) demo thread.          @param Param Unused. */
void IIC_Thread(void *Param);

/** @brief RTOS primitives demo thread.    @param Param Unused. */
void RTOS_Thread(void *Param);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __THREADX_H__ */
