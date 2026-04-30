/**
 * @file http_demo.h
 * @brief Public API for the HTTP demo helper module.
 *
 * Declares the functions implemented by http_demo.c used by demo code to
 * perform simple HTTP requests and handle responses.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __HTTP_DEMO_H__
#define __HTTP_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"
#include "qapi_httpc.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Initialize the HTTP demo client session and DNS.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t http_demo_init();

/**
 * @brief Add an HTTP header field to the pending request.
 * @param[in] type Header name string (e.g., "Content-Type").
 * @param[in] value Header value string.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success.
 */
DAM_Status_t http_demo_header( char *type, char *value);

/**
 * @brief Set the HTTP request body for the current session.
 * @param[in] body Null-terminated body string.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success.
 */
DAM_Status_t http_demo_body(char *body);

/**
 * @brief Establish a TCP connection to the HTTP server.
 * @param[in] URL Server hostname or IPv4 address string.
 * @param[in] port Server port number.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 */
DAM_Status_t HTTP_Connect(uint8_t * URL, uint16_t port);

/**
 * @brief Issue an HTTP request using the current session.
 * @param[in] cmd HTTP method to use (qapi_Net_HTTPc_Method_e).
 * @param[in] URL Request URL or path.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 */
DAM_Status_t HTTP_Request(qapi_Net_HTTPc_Method_e cmd, uint8_t * URL);

/**
 * @brief Shutdown the HTTP demo session and free resources.
 * @return DAM_Status_t DAM_STATUS_SUCCESS.
 */
DAM_Status_t http_demo_shutdown();

#endif




