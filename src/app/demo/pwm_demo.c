/**
 * @file pwm_demo.c
 * @brief PWM demo helpers.
 *
 * Simple PWM initialization and control helpers using the QAPI PWM
 * interfaces. Intended for demo and testing purposes.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "PWM_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "txm_module.h"
#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_txm_base.h"
#include "qapi/qapi_pwm.h"


#include "pwm_demo.h"
#include "../Easylogger/elog.h"

/**
 * @brief Initialize demo PWM peripheral and enable output.
 *
 * Configures a PWM instance with a default frequency and duty cycle
 * and enables the output. Logs errors but does not abort on failure.
 */
 void dam_pwm_init()
 {
	 qapi_PWM_ID_t pwm_id;
	 qapi_Status_t status = QAPI_OK;
	 uint32_t FreqHz;
 
	 status = qapi_PWM_Get_ID(QAPI_PWM_FRAME_0, QAPI_PWM_INSTANCE_1, &pwm_id);
	 if (status == QAPI_OK) {
	 status = qapi_PWM_Set_Frequency(pwm_id, 292);
	 //status = qapi_PWM_Set_Frequency(pwm_id, 960000);
	 if (status != QAPI_OK) {
		 log_i("qapi_PWM_Set_Frequency failed!");
	 // Handle failed case here
	 }
	 status = qapi_PWM_Set_Duty_Cycle(pwm_id, 80);
	 if (status != QAPI_OK) {
		 log_i("qapi_PWM_Set_Duty_Cycle failed!");
	 // Handle failed case here
	 }
	 status = qapi_PWM_Enable(pwm_id, true);
	 if (status != QAPI_OK) {
		 log_i("qapi_PWM_Enable failed!");
	 // Handle failed case here
	 }
	 status = qapi_PWM_Get_Clock_Frequency(pwm_id, &FreqHz);
	 if (status != QAPI_OK) {
		 log_i("qapi_PWM_Get_Clock_Frequency failed!");
	 } else {
		 log_i("Freq=%d", FreqHz);
	 }
	 }
 }
 
 /**
  * @brief Activate PWM output (demo placeholder).
  *
  * This function is a placeholder for application-specific activation
  * logic; the demo uses `dam_pwm_init` to enable hardware directly.
  */
 void dam_pwm_activate()
 {
 
 }
 
	/**
	* @brief Deactivate PWM output (demo placeholder).
	*
	* Disables or stops PWM output when implemented. Currently a no-op.
	*/
 void dam_pwm_deactivate()
 {
 
 }


