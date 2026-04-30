/**
 * @file pwm_demo.h
 * @brief Public prototypes for PWM demo helpers.
 *
 * Declarations for PWM initialization and control helpers used by the
 * demo applications.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __PWM_DEMO_H__
#define __PWM_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/

/**
 * @brief Initialize and enable PWM output for demo use.
 */
void dam_pwm_init();

/**
 * @brief Activate PWM output (application-specific behavior).
 */
void dam_pwm_activate();

/**
 * @brief Deactivate PWM output.
 */
void dam_pwm_deactivate();

#endif

