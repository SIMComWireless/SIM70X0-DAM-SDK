/**
 * @file adc_demo.c
 * @brief ADC demo — helpers for opening/reading/closing ADC channels.
 *
 * This module provides utility wrappers around the QAPI ADC APIs used by
 * the demo applications: open/close the ADC device, query channel
 * properties and perform channel reads. It also includes simple error
 * string helpers and logging helpers used for diagnostics.
 *
 * Example:
 * @code
 *   adc_demo_Open();
 *   adc_demo_Read_Channel(6); // read PA_THERM
 *   adc_demo_Close();
 * @endcode
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)
	#define LOG_TAG    "Adc_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qapi/qapi_adc.h"
#include "qapi/qapi_adc_types.h"
#include "adc_demo.h"
#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

/** Current ADC operation status. */
static qapi_Status_t status;

/** ADC handle obtained from qapi_ADC_Open(). */
static qapi_ADC_Handle_t handle;

/** Cached input properties for the active ADC channel. */
static qapi_Adc_Input_Properties_Type_t Properties;

/** Result buffer filled by qapi_ADC_Read_Channel(). */
static qapi_ADC_Read_Result_t result;

/**
 * @brief Convert an ADC error code to a human-readable string.
 *
 * @param[in] status  QAPI status code to convert.
 *
 * @return Pointer to a static string describing the error.
 *         Returns "UNKNOWN" for unrecognized codes.
 */
char * adc_demo_error_to_string(qapi_Status_t status)
{
    char * err_string;
    switch(status)
    {
        case QAPI_ERROR:
            err_string = "Call failed";
            break;
        case QAPI_ERR_INVALID_PARAM:
            err_string = "Invalid parameters were specified";
            break;
        case QAPI_ERR_NO_MEMORY:
            err_string = "No memory available to support this operation";
            break;
        case QAPI_ERR_NO_RESOURCE:
            err_string = "No more handles are available";
            break;
        default:
            err_string = "UNKNOWN";
            break;
    }
    return err_string;
}

/**
 * @brief Log all fields of an ADC read result.
 *
 * Outputs the status, token, device/channel indices, physical value,
 * percentage, microvolts, and raw code via EasyLogger.
 *
 * @param[in] status  ADC read result structure to display.
 */
void adc_demo_read_status_log(qapi_ADC_Read_Result_t status)
{
    log_i("qapi_ADC_Read_Result_t.eStatus == %d",status.eStatus);
    log_i("qapi_ADC_Read_Result_t.nToken == %d",status.nToken);
    log_i("qapi_ADC_Read_Result_t.nDeviceIdx == %d",status.nDeviceIdx);
    log_i("qapi_ADC_Read_Result_t.nChannelIdx == %d",status.nChannelIdx);
    log_i("qapi_ADC_Read_Result_t.nPhysical == %d",status.nPhysical);
    log_i("qapi_ADC_Read_Result_t.nPercent == %d",status.nPercent);
    log_i("qapi_ADC_Read_Result_t.nMicrovolts == %d",status.nMicrovolts);
    log_i("qapi_ADC_Read_Result_t.nCode == %d",status.nCode);
}

/**
 * @brief Open the ADC device for subsequent channel reads.
 *
 * Acquires an ADC handle from the QAPI layer. Uses a static flag
 * to ensure the device is only opened once, even if called multiple times.
 *
 * @retval DAM_STATUS_SUCCESS  ADC device opened successfully.
 * @retval DAM_STATUS_ERROR    Failed to open ADC device.
 */
DAM_Status_t adc_demo_Open(VOID)
{
    uint32_t id = 0;
	static uint8_t adc_open_flag = 0;
    status = QAPI_ERROR;
	if(!adc_open_flag)
	{
	    status = qapi_ADC_Open(&handle, id);
	    if (status != QAPI_OK)
	    {
	        log_i("open adc failed !!! %s",adc_demo_error_to_string(status));
			return DAM_STATUS_ERROR;
	    }
	    else
	    {
	    	adc_open_flag = 1;
	        log_i("open adc succeeded !!!");
			return DAM_STATUS_SUCCESS;
	    }
	}
}

/**
 * @brief Retrieve input properties for a named ADC channel.
 *
 * Queries the QAPI layer for device and channel index of the given
 * channel name and caches the result in the global @ref Properties.
 *
 * @param[in] Channel_Name  Null-terminated channel name string
 *                          (e.g. "VBATT", "PA_THERM").
 *
 * @retval DAM_STATUS_SUCCESS  Properties retrieved successfully.
 * @retval DAM_STATUS_ERROR    Failed to retrieve channel properties.
 *
 * @see adc_demo_channel_table for the list of valid channel names.
 */
DAM_Status_t adc_demo_Get_Input_Properties(char *Channel_Name)
{
	uint32_t Channel_Name_Size;
    status = QAPI_ERROR;

    log_i("adc_demo_Get_Input_Properties run");
    Channel_Name_Size = strlen (Channel_Name);
    status = qapi_ADC_Get_Input_Properties(handle, Channel_Name, Channel_Name_Size, &Properties);
    if (status != QAPI_OK)
    {
        log_i("get input Properties failed !!! %s",adc_demo_error_to_string(status));
		return DAM_STATUS_ERROR;
    }
    else
    {
        log_i("get input Properties succeeded !!!");
        log_i("channel name is  %s",Channel_Name);
        log_i("nDeviceIdx  =  %d",Properties.nDeviceIdx);
        log_i("nChannelIdx  =  %d",Properties.nChannelIdx);
		return DAM_STATUS_SUCCESS;
    }
}

/**
 * @brief Read the current value from an ADC channel.
 *
 * Sets the channel and device indices in the global @ref Properties,
 * then calls qapi_ADC_Read_Channel() to perform the conversion.
 *
 * @param[in] ChannelIdx  Channel index (0-8). See the channel table
 *                        in the @ref adc_demo.c file header.
 *
 * @retval DAM_STATUS_SUCCESS  Channel read successfully.
 * @retval DAM_STATUS_ERROR    Failed to read the channel.
 *
 * @note The ADC input voltage range is 0 to 1.8V.
 */
DAM_Status_t adc_demo_Read_Channel(uint32_t ChannelIdx)
{
    log_i("adc_demo_Read_Channel run");

    Properties.nChannelIdx = ChannelIdx;
    Properties.nDeviceIdx = 0;
    status = qapi_ADC_Read_Channel(handle, &Properties, &result);
    if (status != QAPI_OK)
    {
        log_i("read channel failed !!! %s",adc_demo_error_to_string(status));
		return DAM_STATUS_ERROR;
    }
    else
    {
        log_i("read channel succeeded !!!");
        adc_demo_read_status_log(result);
		return DAM_STATUS_SUCCESS;
    }
}

/**
 * @brief Close the ADC device and release the handle.
 *
 * @retval DAM_STATUS_SUCCESS  ADC device closed successfully.
 * @retval DAM_STATUS_ERROR    Failed to close ADC device.
 *
 * @see adc_demo_Open
 */
DAM_Status_t adc_demo_Close(VOID)
{
    status = qapi_ADC_Close(handle, false);
    log_i("adc_demo_Close run");
    if (status != QAPI_OK)
    {
        log_i("close adc failed !!! %s",adc_demo_error_to_string(status));
		return DAM_STATUS_ERROR;
    }
    else
    {
        log_i("close adc succeeded !!!");
		return DAM_STATUS_SUCCESS;
    }
}
