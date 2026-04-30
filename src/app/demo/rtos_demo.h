/**
 * @file rtos_demo.h
 * @brief Public prototype for the ThreadX RTOS demo entry.
 *
 * Declares the demo entry function implemented in rtos_demo.c.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __RTOS_DEMO_H__
#define __RTOS_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/

/**
 * @brief Initialize and start the RTOS demo threads and resources.
 * @return int 0 on success; negative on error.
 */
int rtos_demo_entry(void);

#endif

