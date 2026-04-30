/**
 * @file pal_module.c
 * @brief DAM application entry and task creation helpers.
 *
 * Initializes logging, byte pools, semaphores and creates demo threads.
 * Customers may enable specific demo tasks by defining the corresponding
 * preprocessor symbols in this file.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)
#define LOG_TAG                    "DAM_main"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qapi_types.h"
#include "qapi.h"
#include "qapi_status.h"
#include "qapi_uart.h"
#include "qapi_fs.h"
#include "qapi_timer.h"
#include "threadx.h"

#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/
#define HELLO_WORLD_TASK
//#define SOCKET_DEMO_TASK
//#define GPIO_DEMO_TASK
//#define MQTT_DEMO_TASK
//#define AT_DEMO_TASK
//#define ADC_DEMO_TASK
//#define FS_DEMO_TASK
//#define HTTP_DEMO_TASK
//#define GPS_DEMO_TASK
//#define UPDATE_DEMO_TASK
//#define UPDATE_STATE_CHECK_DEMO_TASK
//#define PWM_DEMO_TASK
//#define IIC_DEMO_TASK
//#define RTOS_DEMO_TASK

#define TASK_STACK_SIZE (50*1024)
#define DAM_BYTE_POOL_SIZE 1024*500

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
UCHAR free_memory_dam[DAM_BYTE_POOL_SIZE];
TX_BYTE_POOL * byte_pool_dam;

/*-------------------------------------------------------------------------
 * Semaphore Declarations
 *-----------------------------------------------------------------------*/
TX_SEMAPHORE *Net_Active_semaphore;
TX_SEMAPHORE *GET_Tracking_location_semaphore;
TX_SEMAPHORE *Update_semaphore;



/*-------------------------------------------------------------------------
 * Thread Declarations
 *-----------------------------------------------------------------------*/
#ifdef HELLO_WORLD_TASK
TX_THREAD			* HELLO_WORLD_Thread_Handle;		 
UCHAR			   Hello_world_stack[TASK_STACK_SIZE];
#endif

#ifdef SOCKET_DEMO_TASK
TX_THREAD			* Socket_Thread_Handle;
UCHAR			   Socket_stack[TASK_STACK_SIZE];
#endif

#ifdef GPIO_DEMO_TASK
TX_THREAD			* GPIO_Thread_Handle;
UCHAR			   GPIO_stack[TASK_STACK_SIZE];
#endif

#ifdef MQTT_DEMO_TASK
TX_THREAD			* MQTT_Thread_Handle;
UCHAR			   MQTT_stack[TASK_STACK_SIZE];
#endif

#ifdef AT_DEMO_TASK
TX_THREAD			* AT_Thread_Handle;
UCHAR			   AT_stack[TASK_STACK_SIZE];
#endif

#ifdef ADC_DEMO_TASK
TX_THREAD			* ADC_Thread_Handle;
UCHAR			   ADC_stack[TASK_STACK_SIZE];
#endif

#ifdef FS_DEMO_TASK
TX_THREAD			* FS_Thread_Handle;
UCHAR			   FS_stack[TASK_STACK_SIZE];
#endif

#ifdef HTTP_DEMO_TASK
TX_THREAD			* HTTP_Thread_Handle;
UCHAR			   HTTP_stack[TASK_STACK_SIZE];
#endif

#ifdef GPS_DEMO_TASK
TX_THREAD			* GPS_Thread_Handle;
UCHAR			   GPS_stack[TASK_STACK_SIZE];
#endif

#ifdef UPDATE_DEMO_TASK
TX_THREAD			* UPDATE_Thread_Handle;
UCHAR			   UPDATE_stack[TASK_STACK_SIZE];
#endif

#ifdef UPDATE_STATE_CHECK_DEMO_TASK
TX_THREAD			* UPDATE_State_Check_Thread_Handle;
UCHAR			   UPDATE_State_Check_stack[TASK_STACK_SIZE];
#endif

#ifdef PWM_DEMO_TASK
TX_THREAD			* PWM_Thread_Handle;
UCHAR			   PWM_stack[TASK_STACK_SIZE];
#endif

#ifdef IIC_DEMO_TASK
TX_THREAD			* IIC_Thread_Handle;
UCHAR			   IIC_stack[TASK_STACK_SIZE];
#endif

#ifdef RTOS_DEMO_TASK
TX_THREAD			* RTOS_Thread_Handle;
UCHAR			   RTOS_stack[TASK_STACK_SIZE];
#endif

extern void Uart_Debug_Initialize(void);

/**
 * @brief DAM application start entry point.
 *
 * Performs platform initialization: UART debug setup, allocates the demo
 * byte pool, creates common semaphores and starts configured demo threads.
 *
 * @return int TX_SUCCESS on success; negative on error.
 */
int qcli_dam_app_start(void)
{
   /* Variable definitions */
	int Result;
	UINT status;
	
	/* Code_Initialize */
	Uart_Debug_Initialize();
	
	/* Allocate memory pool */
	txm_module_object_allocate(&byte_pool_dam, sizeof(TX_BYTE_POOL));
    tx_byte_pool_create(byte_pool_dam, "byte pool dam", free_memory_dam, DAM_BYTE_POOL_SIZE);
	log_i("Allocate memory byte pool dam");

	/*Sleep for 100 ms*/
    tx_thread_sleep(10);
	log_i("demo app starts");
	
	/* Initialize the platform.*/
	
	/* Create the semaphores(s) */
	/* definition and creation of Net_Active_semaphore */	
	txm_module_object_allocate(&Net_Active_semaphore, sizeof(TX_SEMAPHORE)); 
	status = tx_semaphore_create(Net_Active_semaphore,"Net_Active_semaphore_name", 0);
	 if(status != TX_SUCCESS)
    {
      log_e("Failed to start Net_Active_semaphore");
    }
	 
	 /* definition and creation of GET_Tracking_location_semaphore */	
	txm_module_object_allocate(&GET_Tracking_location_semaphore, sizeof(TX_SEMAPHORE)); 
	status = tx_semaphore_create(GET_Tracking_location_semaphore,"GET_Tracking_location_semaphore_name", 0);
	 if(status != TX_SUCCESS)
    {
      log_e("Failed to start GET_Tracking_location_semaphore");
    }

	 /* definition and creation of Update_semaphore */	
	txm_module_object_allocate(&Update_semaphore, sizeof(TX_SEMAPHORE)); 
	status = tx_semaphore_create(Update_semaphore,"Update_semaphore_name", 0);
	 if(status != TX_SUCCESS)
    {
      log_e("Failed to start Update_semaphore");
    }
	 

	/* Create the thread(s) */

#ifdef HELLO_WORLD_TASK
	/* definition and creation of HELLO_WORLD_Thread */
	txm_module_object_allocate(&HELLO_WORLD_Thread_Handle, sizeof(TX_THREAD));
    Result = tx_thread_create(HELLO_WORLD_Thread_Handle, "HELLO WORLD Thread", Hello_world_Thread, 152, Hello_world_stack,
                                  TASK_STACK_SIZE, 150, 150, TX_NO_TIME_SLICE, TX_AUTO_START);
	if(Result != TX_SUCCESS)
        {
            log_e("Failed to start HELLO_WORLD thread");
        }
#endif

#ifdef SOCKET_DEMO_TASK
	/* definition and creation of SOCKET_Thread */
	txm_module_object_allocate(&Socket_Thread_Handle, sizeof(TX_THREAD));
    Result = tx_thread_create(Socket_Thread_Handle, "SOCKET Thread", Socket_Thread, 152, Socket_stack,
                                  TASK_STACK_SIZE, 151, 151, TX_NO_TIME_SLICE, TX_AUTO_START);
	if(Result != TX_SUCCESS)
        {
            log_e("Failed to start SOCKET_DEMO thread");
        }
#endif

#ifdef GPIO_DEMO_TASK
		/* definition and creation of GPIO_Thread */
		txm_module_object_allocate(&GPIO_Thread_Handle, sizeof(TX_THREAD));
		Result = tx_thread_create(GPIO_Thread_Handle, "GPIO Thread", GPIO_Thread, 152, GPIO_stack,
									  TASK_STACK_SIZE, 152, 152, TX_NO_TIME_SLICE, TX_AUTO_START);
		if(Result != TX_SUCCESS)
			{
				log_e("Failed to start GPIO_DEMO thread");
			}
#endif


#ifdef MQTT_DEMO_TASK
		/* definition and creation of MQTT_Thread */
		txm_module_object_allocate(&MQTT_Thread_Handle, sizeof(TX_THREAD));
		Result = tx_thread_create(MQTT_Thread_Handle, "MQTT Thread", MQTT_Thread, 152, MQTT_stack,
									  TASK_STACK_SIZE, 153, 153, TX_NO_TIME_SLICE, TX_AUTO_START);
		if(Result != TX_SUCCESS)
			{
				log_e("Failed to start MQTT_DEMO thread");
			}
#endif

#ifdef AT_DEMO_TASK
			/* definition and creation of AT_Thread */
			txm_module_object_allocate(&AT_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(AT_Thread_Handle, "AT Thread", AT_Thread, 152, AT_stack,
										  TASK_STACK_SIZE, 154, 154, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start AT_DEMO thread");
				}
#endif

#ifdef ADC_DEMO_TASK
			/* definition and creation of ADC_Thread */
			txm_module_object_allocate(&ADC_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(ADC_Thread_Handle, "ADC Thread", ADC_Thread, 152, ADC_stack,
										  TASK_STACK_SIZE, 155, 155, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start ADC_DEMO thread");
				}
#endif

#ifdef FS_DEMO_TASK
			/* definition and creation of FS_Thread */
			txm_module_object_allocate(&FS_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(FS_Thread_Handle, "FS Thread", FS_Thread, 152, FS_stack,
										  TASK_STACK_SIZE, 156, 156, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start FS_DEMO thread");
				}
#endif

#ifdef HTTP_DEMO_TASK
			/* definition and creation of HTTP_Thread */
			txm_module_object_allocate(&HTTP_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(HTTP_Thread_Handle, "HTTP Thread", HTTP_Thread, 152, HTTP_stack,
										  TASK_STACK_SIZE, 157, 157, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start HTTP_DEMO thread");
				}
#endif

#ifdef GPS_DEMO_TASK
			/* definition and creation of GPS_Thread */
			txm_module_object_allocate(&GPS_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(GPS_Thread_Handle, "GPS Thread", GPS_Thread, 152, GPS_stack,
										  TASK_STACK_SIZE, 158, 158, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start GPS_DEMO thread");
				}
#endif
	
#ifdef UPDATE_DEMO_TASK
			/* definition and creation of UPDATE_Thread */
			txm_module_object_allocate(&UPDATE_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(UPDATE_Thread_Handle, "UPDATE Thread", UPDATE_Thread, 152, UPDATE_stack,
										  TASK_STACK_SIZE, 159, 159, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start UPDATE_DEMO thread");
				}
#endif
	
#ifdef UPDATE_STATE_CHECK_DEMO_TASK
			/* definition and creation of UPDATE_State_Check_Thread */
			txm_module_object_allocate(&UPDATE_State_Check_Thread_Handle, sizeof(TX_THREAD));
			Result = tx_thread_create(UPDATE_State_Check_Thread_Handle, "UPDATE_State_Check Thread", UPDATE_State_Check_Thread, 152, UPDATE_State_Check_stack,
										  TASK_STACK_SIZE, 160, 160, TX_NO_TIME_SLICE, TX_AUTO_START);
			if(Result != TX_SUCCESS)
				{
					log_e("Failed to start UPDATE_State_Check_DEMO thread");
				}
#endif

#ifdef PWM_DEMO_TASK
				/* definition and creation of PWM_Thread */
				txm_module_object_allocate(&PWM_Thread_Handle, sizeof(TX_THREAD));
				Result = tx_thread_create(PWM_Thread_Handle, "PWM Thread", PWM_Thread, 152, PWM_stack,
											  TASK_STACK_SIZE, 161, 161, TX_NO_TIME_SLICE, TX_AUTO_START);
				if(Result != TX_SUCCESS)
					{
						log_e("Failed to start PWM_DEMO thread");
					}
#endif

#ifdef IIC_DEMO_TASK
				/* definition and creation of IIC_Thread */
				txm_module_object_allocate(&IIC_Thread_Handle, sizeof(TX_THREAD));
				Result = tx_thread_create(IIC_Thread_Handle, "IIC Thread", IIC_Thread, 152, IIC_stack,
											  TASK_STACK_SIZE, 161, 161, TX_NO_TIME_SLICE, TX_AUTO_START);
				if(Result != TX_SUCCESS)
					{
						log_e("Failed to start IIC_DEMO thread");
					}
#endif

#ifdef RTOS_DEMO_TASK
				/* definition and creation of RTOS_Thread */
				txm_module_object_allocate(&RTOS_Thread_Handle, sizeof(TX_THREAD));
				Result = tx_thread_create(RTOS_Thread_Handle, "RTOS Thread", RTOS_Thread, 152, RTOS_stack,
											  TASK_STACK_SIZE, 161, 161, TX_NO_TIME_SLICE, TX_AUTO_START);
				if(Result != TX_SUCCESS)
					{
						log_e("Failed to start RTOS_DEMO thread");
					}
#endif
    
    return(TX_SUCCESS);
}

