/**
 * @file ThreadX.h
 * @brief ThreadX demo thread prototypes and status type.
 *
 * Declares the `DAM_Status_t` enum and all demo thread entry function
 * prototypes used by the application layer.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __THREADX_H__
#define __THREADX_H__

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
 typedef enum 
{
   DAM_STATUS_SUCCESS,
   DAM_STATUS_ERROR,
} DAM_Status_t;

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Hello world thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void Hello_world_Thread(void *Param);

/**
 * @brief Socket demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void Socket_Thread(void * Param);

/**
 * @brief GPIO demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void GPIO_Thread(void *Param);

/**
 * @brief MQTT demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void MQTT_Thread(void * Param);

/**
 * @brief AT command demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void AT_Thread(void * Param);

/**
 * @brief ADC demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void ADC_Thread(void * Param);

/**
 * @brief File system demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void FS_Thread(void * Param);

/**
 * @brief HTTP demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void HTTP_Thread(void * Param);

/**
 * @brief GPS demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void GPS_Thread(void *Param);

/**
 * @brief Application update thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void UPDATE_Thread(void *Param);

/**
 * @brief Update state check thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void UPDATE_State_Check_Thread(void *Param);

/**
 * @brief PWM demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void PWM_Thread(void *Param);

/**
 * @brief I2C/IIC demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void IIC_Thread(void *Param);

/**
 * @brief RTOS demo thread entry.
 * @param[in] Param Thread entry parameter (unused).
 */
void RTOS_Thread(void *Param);

#endif

