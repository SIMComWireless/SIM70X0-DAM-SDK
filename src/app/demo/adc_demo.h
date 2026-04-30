/**
 * @file adc_demo.h
 * @brief ADC demo API declarations.
 *
 * Public function prototypes for the ADC demo module used by the
 * demo applications. See @ref adc_demo.c for details and implementation.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __ADC_DEMO_H__
#define __ADC_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Open and initialize ADC demo resources.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t adc_demo_Open(VOID);

/**
 * @brief Retrieve ADC input properties by channel name.
 * @param[in] Channel_Name Null-terminated channel name string.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t adc_demo_Get_Input_Properties(char *Channel_Name);

/**
 * @brief Read a value from the specified ADC channel index.
 * @param[in] ChannelIdx Zero-based channel index to read.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t adc_demo_Read_Channel(uint32_t ChannelIdx);

/**
 * @brief Close ADC demo and release resources.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t adc_demo_Close(VOID);

#endif

