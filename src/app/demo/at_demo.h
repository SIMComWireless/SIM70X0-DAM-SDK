/**
 * @file at_demo.h
 * @brief Public API for the AT command demo helpers.
 *
 * Declares the exported demo function used to send AT commands via the
 * DAM visual AT interface. See @ref at_demo.c for implementation details.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __AT_DEMO_H__
#define __AT_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Send an AT command over the DAM visual AT channel.
 * @param[in] ATCMD Null-terminated AT command string (no trailing CRLF required).
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR on failure.
 *
 * @note The API appends CRLF automatically. The module registers an
 * output callback to receive asynchronous AT responses.
 */
DAM_Status_t atcmd(uint8_t *ATCMD);

#endif

