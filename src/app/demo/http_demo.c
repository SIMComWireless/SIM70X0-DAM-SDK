/**
 * @file http_demo.c
 * @brief HTTP client demo helpers.
 *
 * Provides a minimal HTTP client wrapper for demo use. Handles session
 * creation, DNS setup, request callbacks and writing response data to
 * the demo filesystem.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "Http_demo"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "http_demo.h"
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
#include "qapi_httpc.h"
#include "qapi_ns_utils.h"

#include "../Easylogger/elog.h"


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern TX_BYTE_POOL *byte_pool_dam;

static int fs_handle;
static qapi_FS_Iter_Handle_t iter_handle;
qapi_Net_HTTPc_handle_t http_demo_handle;


/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/


/**
 * @brief Write HTTP response data to the demo file.
 *
 * Called from the HTTP client callback to append received response
 * chunks to the demo file. If the file handle is invalid this is a no-op.
 *
 * @param[in] value Pointer to a `qapi_Net_HTTPc_Response_t` describing
 *                  the received data chunk.
 */
void http_demo_write_data(void* value)
{
    int length;
    int result;
    qapi_FS_Offset_t actual_offset;
    qapi_Net_HTTPc_Response_t *res_data = value;
    if (fs_handle >= 0)
    {
        result = qapi_FS_Seek(fs_handle, 0, QAPI_FS_SEEK_END_E, &actual_offset);
        if(result == QAPI_OK)
        {
            qapi_FS_Write (fs_handle, (*res_data).data, (*res_data).length, &length);
            if (length != (*res_data).length)
            {
                //QCLI_Printf(qcli_http_handle, "Http demo write data error\n");
            }
        }
    }
}

/**
 * @brief HTTP client state callback.
 *
 * Invoked by the HTTP client session to report progress and provide
 * response chunks. When `state > 0` this function writes chunk data to
 * the demo file; when `state == 0` the transfer is complete.
 *
 * @param[in] arg User-provided payload (unused in demo).
 * @param[in] state HTTP client state (positive while receiving chunks,
 *                  zero when complete).
 * @param[in] value Pointer to a `qapi_Net_HTTPc_Response_t` structure.
 */
void HTTPc_demo_cb(
                                                   void* arg,
                                                   /**< User payload information */
                                                   int32_t state,
                                                   /**< HTTP response state */
                                                   void* value
                                                   /**< HTTP response information */)
{
	qapi_Net_HTTPc_Response_t *temp = (qapi_Net_HTTPc_Response_t *)value;
  	//struct http_client_demo_s *hc = (struct http_client_demo_s *)arg;
	log_i("Http state:%d",state);//lxq
	log_i("=========> http client HEAD response: total size %d, response code %d", temp->length, temp->resp_Code);

   if(state > 0)
    {
        http_demo_write_data(value);
    }
    if(state == 0 )
    {
        log_i("Http demo does not write data");
        qapi_FS_Close (fs_handle);
    }
}

/**
 * @brief Initialize the HTTP demo client session.
 *
 * Starts DNS resolver (if not already started), configures DNS servers
 * and creates a new HTTP client session used by subsequent requests.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success.
 */
DAM_Status_t http_demo_init()
{
    
    if(0 == qapi_Net_DNSc_Is_Started())
    {
        qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
    }
    qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
    qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);

    qapi_Net_HTTPc_Start();

     http_demo_handle = qapi_Net_HTTPc_New_sess(
                        60000,
                        0,
                        HTTPc_demo_cb,
                        NULL,
                        256,
                        512);
	qapi_Net_HTTPc_Pass_Pool_Ptr(http_demo_handle, byte_pool_dam);
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Add a header field to the current HTTP session request.
 *
 * @param[in] type Header name (e.g. "Content-Type").
 * @param[in] value Header value string.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS.
 */
DAM_Status_t http_demo_header( char *type, char *value)
{
    qapi_Net_HTTPc_Add_Header_Field(http_demo_handle, (char *)type, (char *)value);
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Set the request body for the current HTTP session.
 *
 * @param[in] body Null-terminated body string.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS.
 */
DAM_Status_t http_demo_body(char *body)
{
    
    qapi_Net_HTTPc_Set_Body(http_demo_handle, (char *)body, strlen(body));
    return DAM_STATUS_SUCCESS;
}

/**
 * @brief Connect the HTTP session to a server.
 *
 * @param[in] URL Server hostname or IPv4 address string.
 * @param[in] port Server port number.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 */
DAM_Status_t HTTP_Connect(uint8_t * URL, uint16_t port)
{
  	int error = QAPI_OK;
	
	log_i("Http IP:%s,port:%d!", URL, port);
	error = qapi_Net_HTTPc_Connect( http_demo_handle, (char*)URL, port);
  	if (error) 
	  {	     
	    log_i("http client connect failed %d\n",error);
		return DAM_STATUS_ERROR;
	  }
	else
	  {
	 	log_i("http client connect success");
	    return DAM_STATUS_SUCCESS;
	  }
}

/**
 * @brief Issue an HTTP request using the current session.
 *
 * Creates a directory and file under "/http_demo" to store the response
 * and performs the HTTP request.
 *
 * @param[in] cmd HTTP method to use (qapi_Net_HTTPc_Method_e).
 * @param[in] URL Request URL or path.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 */
DAM_Status_t HTTP_Request(qapi_Net_HTTPc_Method_e cmd, uint8_t * URL)
{
	int error = QAPI_OK;


	log_i("req_cmd %d\n",cmd);

	if(QAPI_OK != qapi_FS_Iter_Open ("/http_demo", &iter_handle))
   	{
   	    qapi_FS_Mk_Dir ("/http_demo", 0x777);
		return DAM_STATUS_ERROR;
   	}
   	qapi_FS_Iter_Close(iter_handle);
   	qapi_FS_Unlink ("/http_demo/http_demo_test.html");
   	qapi_FS_Open ("/http_demo/http_demo_test.html", QAPI_FS_O_CREAT_E | QAPI_FS_O_TRUNC_E | QAPI_FS_O_RDWR_E, &fs_handle);
   	if(0 == qapi_Net_HTTPc_Request(http_demo_handle, cmd, (char*)URL))
	   	{
	   	    log_i("Http demo request Success!");
			return DAM_STATUS_SUCCESS;	   
	   	}
   	else
	   	{
	   	    log_i("Http demo request Failed!");
	   	    qapi_FS_Close (fs_handle);
			return DAM_STATUS_ERROR;
	   	}
	  
}

/**
 * @brief Disconnect and free the HTTP demo session.
 *
 * Disconnects the HTTP client session, frees the allocated session and
 * stops the HTTP client subsystem.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS.
 */
DAM_Status_t http_demo_shutdown()
{
    qapi_Net_HTTPc_Disconnect(http_demo_handle);
    qapi_Net_HTTPc_Free_sess(http_demo_handle);
    qapi_Net_HTTPc_Stop();
    return DAM_STATUS_SUCCESS;
}

