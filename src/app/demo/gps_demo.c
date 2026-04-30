/**
 * @file gps_demo.c
 * @brief GNSS/GPS demo helpers (tracking, batching, geofences).
 *
 * Implements callback handlers and convenience APIs for starting/stopping
 * tracking and batching, managing geofences and reporting location events
 * to the demo application. Uses QAPI location APIs under the hood.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "GPS_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_location.h"

#include "gps_demo.h"

#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
/* Define the Max number of S_userBuffer.*/ 
#define S_USER_BUFFER_SIZE 4096
	
/* Define a userBuffer,used in gps_demo_init */ 
static uint8_t S_userBuffer[S_USER_BUFFER_SIZE];

static qapi_Location_Error_t s_gps_status;
static uint32_t s_pSessionId_track=0;
static uint32_t s_pSessionId_batch=0;
static int s_gps_init;
static int s_gps_track;
static int s_gps_batch;
static qapi_Location_Callbacks_t s_pCallbacks={0};
static qapi_loc_client_id pClientId[1]={0};

extern TX_SEMAPHORE *GET_Tracking_location_semaphore;

/**
 * @brief Convert GPS API status to human-readable string.
 *
 * @param[in] status QAPI location/status code to convert.
 * @return char* Pointer to a static string describing the status.
 */
char * gps_demo_error_to_string(qapi_Status_t status)
{
    char * err_string;
    switch(status)
    {
        case QAPI_LOCATION_ERROR_GENERAL_FAILURE:
            err_string = "QAPI_LOCATION_ERROR_GENERAL_FAILURE";
            break;
        case QAPI_LOCATION_ERROR_CALLBACK_MISSING:
            err_string = "QAPI_LOCATION_ERROR_CALLBACK_MISSING";
            break;
        case QAPI_LOCATION_ERROR_INVALID_PARAMETER:
            err_string = "QAPI_LOCATION_ERROR_INVALID_PARAMETER";
            break;
        case QAPI_LOCATION_ERROR_ID_EXISTS:
            err_string = "QAPI_LOCATION_ERROR_ID_EXISTS";
            break;
        case QAPI_LOCATION_ERROR_ID_UNKNOWN:
            err_string = "QAPI_LOCATION_ERROR_ID_UNKNOWN";
            break;
        case QAPI_LOCATION_ERROR_ALREADY_STARTED:
            err_string = "QAPI_LOCATION_ERROR_ALREADY_STARTED";
            break;
        case QAPI_LOCATION_ERROR_NOT_INITIALIZED:
            err_string = "QAPI_LOCATION_ERROR_NOT_INITIALIZED";
            break;
        case QAPI_LOCATION_ERROR_GEOFENCES_AT_MAX:
            err_string = "QAPI_LOCATION_ERROR_GEOFENCES_AT_MAX";
            break;
        default:
            err_string = "UNKNOWN";
            break;
    }
    return err_string;
}

static char s_location_str[128];
static char s_lon_int[30];
static char s_lon_flo[30];
static char s_lat_int[30];
static char s_lat_flo[30];

/**
 * @brief Tracking callback invoked when a new location fix is available.
 *
 * Formats latitude/longitude into a string and signals the
 * `GET_Tracking_location_semaphore` to inform waiting tasks.
 *
 * @param[in] location Location information provided by QAPI.
 */
void trackingCb_demo(qapi_Location_t location)
{
    sprintf(s_lat_int,"%d",(int)(location.latitude));
	sprintf(s_lat_flo,"%d",abs(location.latitude*1000000)-abs((int)location.latitude*1000000));
    sprintf(s_lon_int,"%d",(int)(location.longitude));
	sprintf(s_lon_flo,"%d",abs(location.longitude*1000000)-abs((int)location.longitude*1000000));
	sprintf(s_location_str,"lat:%s.%s,lon:%s.%s",s_lat_int,s_lat_flo,s_lon_int,s_lon_flo);
	log_i( "+GNSSINF_track: %s",s_location_str);
	tx_semaphore_put(GET_Tracking_location_semaphore);
}

/**
 * @brief Batching callback invoked with multiple locations.
 *
 * Logs the number of batched locations and outputs the first
 * location in a readable format.
 *
 * @param[in] count Number of locations provided.
 * @param[in] location Pointer to the first qapi_Location_t in the batch.
 */
void batchingCb_demo(size_t count,qapi_Location_t* location)
{
    sprintf(s_lat_int,"%d",(int)(location->latitude));
	sprintf(s_lat_flo,"%d",abs(location->latitude*1000000)-abs((int)location->latitude*1000000));
    sprintf(s_lon_int,"%d",(int)(location->longitude));
	sprintf(s_lon_flo,"%d",abs(location->longitude*1000000)-abs((int)location->longitude*1000000));
	sprintf(s_location_str,"lat:%s.%s,lon:%s.%s",s_lat_int,s_lat_flo,s_lon_int,s_lon_flo);
    log_i("GNSS batching count:%d" , count);
	log_i("+GNSSINF_batch: %s",s_location_str);
}

/**
 * @brief Capability change callback.
 *
 * Reports available GNSS/location capabilities.
 *
 * @param[in] capabilitiesMask Bitmask of reported capabilities.
 */
void capabilitiesCb_demo(uint16_t capabilitiesMask)
{
    log_i( "capabilitiesCb_demo:%x" , capabilitiesMask);
}

/**
 * @brief Geofence breach callback.
 *
 * Invoked when a geofence breach occurs. This demo only logs the event.
 *
 * @param[in] count Number of breached geofences in this event.
 * @param[in] ids Array of geofence IDs.
 * @param[in] location Location at breach time.
 * @param[in] breachType Type of breach event.
 * @param[in] timestamp Timestamp of the event.
 */
void geofenceBreachCb_demo(size_t count,uint32_t* ids,qapi_Location_t location,qapi_Geofence_Breach_t breachType,uint64_t timestamp)
{
    log_i( "geofenceBreachCb_demo !!!");
}

/**
 * @brief Collective response callback.
 *
 * Handles multi-response operations such as geofence batch requests.
 *
 * @param[in] count Number of responses.
 * @param[in] err Array of error codes corresponding to responses.
 * @param[in] ids Array of IDs for which responses were received.
 */
void collectiveResponseCb_demo(size_t count,qapi_Location_Error_t* err,uint32_t* ids)
{
	log_i( "collectiveResponseCb_demo !!!");
}

/**
 * @brief Single-response callback.
 *
 * Invoked to indicate the success or failure of a specific location
 * operation identified by `id`.
 *
 * @param[in] err Error code for the response.
 * @param[in] id Identifier associated with the response.
 */
void responseCb_demo(qapi_Location_Error_t err,uint32_t id)
{
	if (err != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "responseCb_demo failed !!! %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "responseCb_demo succeeded !!!");
    }
}

/**
 * @brief Initialize the GPS/location demo client.
 *
 * Registers callbacks and allocates the user buffer for the location
 * client. Calls `qapi_Loc_Init` and `qapi_Loc_Set_User_Buffer`.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_init(VOID)
{
    
	s_gps_status = QAPI_LOCATION_ERROR_SUCCESS;
	
	memset(&s_pCallbacks, 0, sizeof(s_pCallbacks));
    s_pCallbacks.size = sizeof(s_pCallbacks);
	
    s_pCallbacks.trackingCb=(qapi_Tracking_Callback)trackingCb_demo;//qapi_Tracking_Callback;
    s_pCallbacks.batchingCb=(qapi_Batching_Callback)batchingCb_demo;
    s_pCallbacks.capabilitiesCb=(qapi_Capabilities_Callback)capabilitiesCb_demo;
	s_pCallbacks.geofenceBreachCb=(qapi_Geofence_Breach_Callback)geofenceBreachCb_demo;
	s_pCallbacks.collectiveResponseCb=(qapi_Collective_Response_Callback)collectiveResponseCb_demo;
	s_pCallbacks.responseCb=(qapi_Response_Callback)responseCb_demo;

    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Init(pClientId,&s_pCallbacks);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "init gps failed !!! %s",gps_demo_error_to_string(s_gps_status));
    }
    else
    {
        log_i( "init gps succeeded !!!");
    }

	s_gps_status = (qapi_Location_Error_t)qapi_Loc_Set_User_Buffer(
            		(qapi_loc_client_id)pClientId[0],
            		(uint8_t*)S_userBuffer,
            		(size_t)S_USER_BUFFER_SIZE);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
	{
        log_e("Set user buffer failed ! (ret %d)", s_gps_status);
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Perform a single-shot tracking test.
 *
 * Starts tracking with the provided minimum interval/distance and waits
 * for a location to arrive or times out.
 *
 * @param[in] minInterval Minimum interval between fixes (ms or unit used by API).
 * @param[in] minDistance Minimum distance between fixes.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS if a location was received; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t tracking_test_singleshot(uint32_t minInterval,uint32_t minDistance)
{

    int result=0;
	
    log_i("GPS TRACKING TEST : SINGLESHOT : START");


    gps_demo_start_tracking(minInterval,minDistance);

    log_i("Waiting for Location..");
	result = tx_semaphore_get(GET_Tracking_location_semaphore, 2000);//wait for 20s
    gps_demo_stop_tracking();
	if (result == TX_SUCCESS)
		{
		log_i("Got Location");
		log_i("GPS TRACKING TEST : SINGLESHOT : END");
		return DAM_STATUS_SUCCESS;
		}
	else
		{
		log_e("fail to Get Location");
		log_e("GPS TRACKING TEST : SINGLESHOT : END");
		return DAM_STATUS_ERROR;
		}    
    
}
		
/**
 * @brief Start continuous GPS tracking.
 *
 * @param[in] minInterval Minimum interval between fixes.
 * @param[in] minDistance Minimum distance between fixes.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 */
DAM_Status_t gps_demo_start_tracking(uint32_t minInterval,uint32_t minDistance)
{
    qapi_Location_Options_t pOptions;

	pOptions.minInterval=minInterval;
	pOptions.minDistance=minDistance;	
	pOptions.size=sizeof(qapi_Location_Options_t);
	
    if (s_gps_track == true)
    {
        log_i( "gps is already tracking");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Start_Tracking(pClientId[0],&pOptions,&s_pSessionId_track);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "qapi_Loc_Start_Tracking failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "qapi_Loc_Start_Tracking succeeded !!!");
		s_gps_track= true;
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Stop ongoing GPS tracking.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR if tracking was not active.
 */
DAM_Status_t gps_demo_stop_tracking(VOID)
{	
    if (s_gps_track== false)
    {
        log_i( "gps not in tracking, please tracking first");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Stop_Tracking(pClientId[0],s_pSessionId_track);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "stop tracking failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "stop tracking succeeded !!!");
		s_gps_track=false;
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Update tracking options for an active tracking session.
 *
 * @param[in] minInterval New minimum interval between fixes.
 * @param[in] minDistance New minimum distance between fixes.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_update_tracking_option(uint32_t minInterval,uint32_t minDistance)
{	
    
	qapi_Location_Options_t pOptions;
	    
    pOptions.minInterval = minInterval;   
    pOptions.minDistance = minDistance;
    pOptions.size=sizeof(qapi_Location_Options_t);
	log_i( "gps minInterval=%d,minDistance=%d", pOptions.minInterval, pOptions.minDistance);
	
	if (s_gps_track== false)
    {
        log_e( "gps not in tracking, please tracking first");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Update_Tracking_Options(pClientId[0],s_pSessionId_track,&pOptions);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "update tracking session failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "update tracking session succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Start GNSS batching mode.
 *
 * @param[in] minInterval Minimum interval for batched fixes.
 * @param[in] minDistance Minimum distance for batched fixes.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_start_batching(uint32_t minInterval,uint32_t minDistance)
{
    qapi_Location_Options_t pOptions;
	
	pOptions.minInterval=minInterval;
	pOptions.minDistance=minDistance;	
	pOptions.size=sizeof(qapi_Location_Options_t);
	
    if (s_gps_batch == true)
    {
        log_i( "gps is already batching");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Start_Batching(pClientId[0],&pOptions,&s_pSessionId_batch);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "gps_demo_start_batching failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "gps_demo_start_batching succeeded !!!");
		s_gps_batch= true;
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Stop GNSS batching mode.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_stop_batching(VOID)
{	
    if (s_gps_batch== false)
    {
        log_e( "gps not in batching, please batching first");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Stop_Batching(pClientId[0],s_pSessionId_batch);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "stop batching failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "stop batching succeeded !!!");
		s_gps_batch=false;
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Update batching options for an active batching session.
 *
 * @param[in] minInterval New minimum interval for batched fixes.
 * @param[in] minDistance New minimum distance for batched fixes.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_update_batching_option(uint32_t minInterval,uint32_t minDistance)
{	
	qapi_Location_Options_t pOptions;
	    
    pOptions.minInterval = minInterval;
    pOptions.minDistance = minDistance;    
    pOptions.size=sizeof(qapi_Location_Options_t);
	log_i( "gps minInterval=%d,minDistance=%d", pOptions.minInterval, pOptions.minDistance);
	
	if (s_gps_batch== false)
    {
        log_e( "gps not in batching, please tracking first");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Update_Batching_Options(pClientId[0],s_pSessionId_batch,&pOptions);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "update batching session failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "update batching session succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Retrieve a number of batched locations.
 *
 * @param[in] count Number of batched locations to retrieve.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_get_batching_loc(size_t count)
{	

	if (s_gps_batch== false)
    {
        log_e( "gps not in batching, please batching first");
        return DAM_STATUS_ERROR;
    }
	
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Get_Batched_Locations(pClientId[0],s_pSessionId_batch,count);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "get batching data failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "get batching data succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}


static uint32_t* s_pIdArray_geo[10];
static int s_geo_num=0;

/**
 * @brief Add a geofence.
 *
 * @param[in] latitude Geofence center latitude.
 * @param[in] longitude Geofence center longitude.
 * @param[in] radius Geofence radius in meters.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_Add_Geofences(double latitude, double longitude,double radius)
{	

    qapi_Geofence_Option_t pOptions;
	qapi_Geofence_Info_t pData;

	pOptions.size=sizeof(qapi_Geofence_Option_t);
	pOptions.breachTypeMask=10;
	pOptions.dwellTime=1;
	pOptions.responsiveness=500;
	
	pData.latitude=latitude;
	pData.longitude=longitude;
    pData.radius=radius;
	pData.size=sizeof(qapi_Geofence_Info_t);
	
	
	s_geo_num+=1;
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Add_Geofences(pClientId[0],s_geo_num,&pOptions,&pData,&s_pIdArray_geo);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "geo add failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "geo add succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Remove the most recently added geofence (demo helper).
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_remove_Geofences(VOID)
{	
	if(s_geo_num== 0)
	{
        log_e( "geo not exist, please init it");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Remove_Geofences(pClientId[0],s_geo_num,s_pIdArray_geo);
	s_geo_num-=1;
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "geo remove failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "geo remove succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Pause all configured geofences.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_pause_Geofences(VOID)
{	
	if(s_geo_num== 0)
	{
        log_e( "geo not exist, please init it");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Pause_Geofences(pClientId[0],s_geo_num,s_pIdArray_geo);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "geo pause failed : %se",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "geo pause succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Resume previously paused geofences.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_resume_Geofences(VOID)
{	
	if(s_geo_num== 0)
	{
        log_e( "geo not exist, please init it");
        return DAM_STATUS_ERROR;
    }
    s_gps_status = (qapi_Location_Error_t)qapi_Loc_Resume_Geofences(pClientId[0],s_geo_num,s_pIdArray_geo);
    if (s_gps_status != QAPI_LOCATION_ERROR_SUCCESS)
    {
        log_e( "geo resume failed : %s",gps_demo_error_to_string(s_gps_status));
	}
    else
    {
        log_i( "geo resume succeeded !!!");
    }
    return DAM_STATUS_SUCCESS;
}

