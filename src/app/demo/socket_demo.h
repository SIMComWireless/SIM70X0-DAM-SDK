/**
 * @file socket_demo.h
 * @brief Public socket demo prototypes.
 *
 * Declares the TCP/UDP demo helper functions implemented in socket_demo.c.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __SOCKET_DEMO_H__
#define __SOCKET_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Perform a blocking TCP socket demo transaction.
 * @param[in] url Server hostname or IP string.
 * @param[in] port Server port number.
 * @param[in] data Payload to send after connection (may be NULL).
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t socket_demo_TCP(char *url,int port,char *data);

/**
 * @brief Perform a UDP socket demo transaction.
 * @param[in] url Destination hostname or IP string.
 * @param[in] port Destination port number.
 * @param[in] data Payload to send (may be NULL).
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t socket_demo_UDP(char *url,int port,char *data);

#endif

