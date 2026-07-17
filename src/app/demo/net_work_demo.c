/**
 * @file net_work_demo.c
 * @brief Network management and DSS helpers for demos.
 *
 * Handles DSS callbacks, PDN setup and simple network bring-up used by
 * demo applications. Provides a convenience wrapper to start a data call
 * with provided APN and performs basic DNS configuration.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

 #if !defined(LOG_TAG)    
	#define LOG_TAG    "Net_demo"
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "qapi.h"
#include "qapi_status.h"
#include "qapi_types.h"
#include "qapi_fs.h"
#include "qapi_timer.h"
#include "msgcfg.h"
#include "msg_mask.h"
#include "qapi_diag.h"
#include "qapi_dss.h"
#include "qapi_socket.h"
#include "qapi_dnsc.h"
#include "qapi_ns_utils.h"
#include "qapi_ssl.h"
#include "txm_module.h"

#include "../demo/netutils.h"
#include "net_work_demo.h"
#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern TX_BYTE_POOL * byte_pool_dam;
extern TX_SEMAPHORE *Net_Active_semaphore;
char g_APN[50] = {0};
qapi_DSS_Net_Evt_t socket_last_evt;
static qapi_DSS_Hndl_t NW_dss_handle;
static bool dss_initialized = false;

/**
  * @brief  DSS event error code print string.
  * @param  event --string match according to status code.
  * @note   
  * @retval err_string --string for status code.
  */
char * DSS_event_to_String(qapi_DSS_Net_Evt_t event)
{
    char * err_string;
    switch(event)
    {
        case QAPI_DSS_EVT_INVALID_E:
            err_string = "QAPI_DSS_EVT_INVALID_E";
            break;
        case QAPI_DSS_EVT_NET_IS_CONN_E:
            err_string = "QAPI_DSS_EVT_NET_IS_CONN_E";
            break;
        case QAPI_DSS_EVT_NET_NO_NET_E:
            err_string = "QAPI_DSS_EVT_NET_NO_NET_E";
            break;
        case QAPI_DSS_EVT_NET_RECONFIGURED_E:
            err_string = "QAPI_DSS_EVT_NET_RECONFIGURED_E";
            break;
        case QAPI_DSS_EVT_NET_NEWADDR_E:
            err_string = "QAPI_DSS_EVT_NET_NEWADDR_E";
            break;
        case QAPI_DSS_EVT_NET_DELADDR_E:
            err_string = "QAPI_DSS_EVT_NET_DELADDR_E";
            break;
        case QAPI_DSS_EVT_NIPD_DL_DATA_E:
            err_string = "QAPI_DSS_EVT_NIPD_DL_DATA_E";
            break;

        default:
            err_string = "Unknown";
            break;
    }
    return err_string;
}

/**
 * @brief DSS event callback for network status changes.
 *
 * Handles PDN connect/disconnect events, configures DNS servers when
 * the PDN comes up, and signals `Net_Active_semaphore` for demos waiting
 * on network availability.
 *
 * @param[in] hndl Handle associated with the DSS event.
 * @param[in] user_data User-provided pointer (unused in demo).
 * @param[in] evt Event identifier describing the network event.
 * @param[in] payload_ptr Additional event-specific payload information.
 */
static void DAM_NW_dss_cb(
    qapi_DSS_Hndl_t                 hndl,                /* Handle for which this event is associated */
    void                                     *user_data,         /* Application-provided user data */
    qapi_DSS_Net_Evt_t          evt,                     /* Event identifier */
    qapi_DSS_Evt_Payload_t *payload_ptr      /* Associated event information */
)
{
    qapi_DSS_Addr_Info_t *info_ptr = NULL;
    unsigned int len = 0;
    char dp_v4[16];
    char ds_v4[16];
	unsigned int IPV4=0;

	log_i("DSS evt->%s" , DSS_event_to_String(evt));

    if(evt == QAPI_DSS_EVT_NET_IS_CONN_E)
    {
        if(0 == qapi_Net_DNSc_Is_Started())
        {
            qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
			log_i("dns is starting");
        }

        if (qapi_DSS_Get_IP_Addr_Count(NW_dss_handle, &len) == QAPI_OK)
        {
            tx_byte_allocate(byte_pool_dam, (VOID **) &info_ptr, (sizeof(qapi_DSS_Addr_Info_t)*len), TX_NO_WAIT);
            memset(info_ptr, 0, (sizeof(qapi_DSS_Addr_Info_t)*len));
            if (qapi_DSS_Get_IP_Addr(NW_dss_handle,info_ptr,len) == QAPI_OK)
            {
                if((NULL != inet_ntop(AF_INET,&info_ptr->dnsp_addr_s.addr.v4,dp_v4,sizeof(dp_v4)))
                    &&(NULL != inet_ntop(AF_INET,&info_ptr->dnss_addr_s.addr.v4,ds_v4,sizeof(ds_v4))))
                {
                    if((0 != qapi_Net_DNSc_Add_Server(dp_v4, QAPI_NET_DNS_ANY_SERVER_ID) )
                    	|| (0 !=qapi_Net_DNSc_Add_Server(ds_v4, QAPI_NET_DNS_ANY_SERVER_ID)))
                    {
                    		qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                    		qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);                    }
                }
                else
                {
                    qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                    qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
                }
            }
            else
            {
                qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
            }
        }
        else
        {
            qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
            qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
        }
		if (info_ptr != NULL)
		{
            tx_byte_release((VOID *) info_ptr);  	
        }
		
	    if (evt != socket_last_evt) {
	    log_i("active network");
		tx_semaphore_put(Net_Active_semaphore);
		
        }
    }
    else
    {
        if (evt != socket_last_evt) {
            //Debug_Printf("not active network\r\n");
        }
        if(NW_dss_handle != NULL)
            qapi_DSS_Rel_Data_Srvc_Hndl(NW_dss_handle);
        NW_dss_handle = NULL;
        DAM_NW_dss_init(g_APN);
        
    }
    socket_last_evt = evt;
}

/**
 * @brief Initialize DSS and start a data call using the provided APN.
 *
 * Sets up the DSS service, assigns the demo byte pool, configures call
 * parameters (IP version, radio tech), and initiates the data call.
 *
 * @param[in] APN APN string to use for the PDN; if NULL, APN is cleared.
 * @return bool true on success (data call started); false on failure.
 */
bool DAM_NW_dss_init(char * APN)
{
    int result = 0;
    qapi_DSS_Call_Param_Value_t param_info;

    if (!dss_initialized) {
        qapi_DSS_Init(QAPI_DSS_MODE_GENERAL);
        dss_initialized = true;
    }

	if (qapi_DSS_Get_Data_Srvc_Hndl(DAM_NW_dss_cb, NULL, &NW_dss_handle) == QAPI_OK)
  	{
	    if(qapi_DSS_Pass_Pool_Ptr(NW_dss_handle,byte_pool_dam) != QAPI_OK)
	    {
	      log_e(" pass pool api failed");
          return false;
	    }    
  	}	 
 
    if ( NW_dss_handle != NULL )
    {   
        param_info.buf_val = NULL;
        param_info.num_val = QAPI_DSS_RADIO_TECH_UNKNOWN;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_TECH_PREF_E, &param_info);
 
        param_info.buf_val = NULL;
        param_info.num_val = QAPI_DSS_IP_VERSION_6;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_IP_VERSION_E, &param_info);
 
        param_info.buf_val = NULL;
        param_info.num_val = 1;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_UMTS_PROFILE_IDX_E, &param_info);

        if (APN != NULL)
        {
            strncpy(g_APN, APN, sizeof(g_APN) - 1);
            g_APN[sizeof(g_APN) - 1] = '\0';
            param_info.buf_val = APN;
            param_info.num_val = strlen(APN);
            result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle,
                       QAPI_DSS_CALL_INFO_APN_NAME_E, &param_info);
        }
	    else
        {
            log_e("APN not set");
            memset(g_APN, 0x00, sizeof(g_APN));
        }
    }
    else
    {
        log_e("NW_dss_handle is NULL");
        return false;
    }
 
    result = qapi_DSS_Start_Data_Call ( NW_dss_handle);
    return result;
}



