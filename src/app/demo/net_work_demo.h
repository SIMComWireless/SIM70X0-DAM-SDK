/**
 * @file net_work_demo.h
 * @brief Public API for demo network helpers.
 *
 * Declares the network initialization wrapper used to start the data
 * service (DSS) for demo applications.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __NET_WORK_DEMO_H__
#define __NET_WORK_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Initialize the DSS data service and start a data call.
 * @param[in] APN Access Point Name string to use for the PDN; may be NULL to clear.
 * @return bool true if the data call was started successfully; false otherwise.
 */
bool DAM_NW_dss_init(char * APN);

#endif

