/**
 * @file pal_module.c
 * @brief DAM application entry point and task creation.
 *
 * Initialises logging, byte pools, semaphores and creates demo threads.
 * Enable specific demo tasks by un-commenting the corresponding
 * @c \#define symbols below.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)
#define LOG_TAG                    "DAM_main"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                XX_LOG_LVL
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
 * Task Enable Switches
 *
 * Un-comment a \#define to include the corresponding demo thread.
 *-----------------------------------------------------------------------*/
//#define HELLO_WORLD_TASK
//#define SOCKET_DEMO_TASK
//#define GPIO_DEMO_TASK
//#define MQTT_DEMO_TASK
#define AT_DEMO_TASK
//#define ADC_DEMO_TASK
//#define FS_DEMO_TASK
//#define HTTP_DEMO_TASK
//#define GPS_DEMO_TASK
//#define UPDATE_DEMO_TASK
//#define UPDATE_STATE_CHECK_DEMO_TASK
//#define PWM_DEMO_TASK
//#define IIC_DEMO_TASK
//#define RTOS_DEMO_TASK

/*-------------------------------------------------------------------------
 * Per-Task Stack Size and Priority Configuration
 *
 * Stack sizes are in bytes.  Priority values: lower number = higher
 * priority in ThreadX (valid range 0 – 255).
 *-----------------------------------------------------------------------*/
#define HELLO_WORLD_STACK_SIZE           (4  * 1024)
#define HELLO_WORLD_PRIORITY             180

#define SOCKET_STACK_SIZE                (12 * 1024)
#define SOCKET_PRIORITY                  151

#define GPIO_STACK_SIZE                  (8  * 1024)
#define GPIO_PRIORITY                    152

#define MQTT_STACK_SIZE                  (12 * 1024)
#define MQTT_PRIORITY                    153

#define AT_STACK_SIZE                    (8  * 1024)
#define AT_PRIORITY                      154

#define ADC_STACK_SIZE                   (4  * 1024)
#define ADC_PRIORITY                     155

#define FS_STACK_SIZE                    (8  * 1024)
#define FS_PRIORITY                      156

#define HTTP_STACK_SIZE                  (12 * 1024)
#define HTTP_PRIORITY                    157

#define GPS_STACK_SIZE                   (8  * 1024)
#define GPS_PRIORITY                     158

#define UPDATE_STACK_SIZE                (8  * 1024)
#define UPDATE_PRIORITY                  159

#define UPDATE_STATE_CHECK_STACK_SIZE    (4  * 1024)
#define UPDATE_STATE_CHECK_PRIORITY      160

#define PWM_STACK_SIZE                   (4  * 1024)
#define PWM_PRIORITY                     162

#define IIC_STACK_SIZE                   (8  * 1024)
#define IIC_PRIORITY                     163

#define RTOS_STACK_SIZE                  (4  * 1024)
#define RTOS_PRIORITY                    164

/** @brief Size of the DAM shared byte pool (128 KiB). */
#define DAM_BYTE_POOL_SIZE               (1024 * 128)

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

/** @brief Raw memory backing the DAM byte pool. */
UCHAR free_memory_dam[DAM_BYTE_POOL_SIZE];

/** @brief DAM shared byte pool handle. */
TX_BYTE_POOL *byte_pool_dam;

/*-------------------------------------------------------------------------
 * Semaphore Declarations
 *-----------------------------------------------------------------------*/

/** @brief Signalled when the data-network PDN becomes active. */
TX_SEMAPHORE *Net_Active_semaphore;

/** @brief Signalled when a GPS tracking fix is available. */
TX_SEMAPHORE *GET_Tracking_location_semaphore;

/** @brief Signalled when an OTA download completes. */
TX_SEMAPHORE *Update_semaphore;

/*-------------------------------------------------------------------------
 * Thread Declarations
 * Each task has its own stack array and thread-pointer handle.
 *-----------------------------------------------------------------------*/
#ifdef HELLO_WORLD_TASK
TX_THREAD  *HELLO_WORLD_Thread_Handle;
UCHAR       Hello_world_stack[HELLO_WORLD_STACK_SIZE];
#endif

#ifdef SOCKET_DEMO_TASK
TX_THREAD  *Socket_Thread_Handle;
UCHAR       Socket_stack[SOCKET_STACK_SIZE];
#endif

#ifdef GPIO_DEMO_TASK
TX_THREAD  *GPIO_Thread_Handle;
UCHAR       GPIO_stack[GPIO_STACK_SIZE];
#endif

#ifdef MQTT_DEMO_TASK
TX_THREAD  *MQTT_Thread_Handle;
UCHAR       MQTT_stack[MQTT_STACK_SIZE];
#endif

#ifdef AT_DEMO_TASK
TX_THREAD  *AT_Thread_Handle;
UCHAR       AT_stack[AT_STACK_SIZE];
#endif

#ifdef ADC_DEMO_TASK
TX_THREAD  *ADC_Thread_Handle;
UCHAR       ADC_stack[ADC_STACK_SIZE];
#endif

#ifdef FS_DEMO_TASK
TX_THREAD  *FS_Thread_Handle;
UCHAR       FS_stack[FS_STACK_SIZE];
#endif

#ifdef HTTP_DEMO_TASK
TX_THREAD  *HTTP_Thread_Handle;
UCHAR       HTTP_stack[HTTP_STACK_SIZE];
#endif

#ifdef GPS_DEMO_TASK
TX_THREAD  *GPS_Thread_Handle;
UCHAR       GPS_stack[GPS_STACK_SIZE];
#endif

#ifdef UPDATE_DEMO_TASK
TX_THREAD  *UPDATE_Thread_Handle;
UCHAR       UPDATE_stack[UPDATE_STACK_SIZE];
#endif

#ifdef UPDATE_STATE_CHECK_DEMO_TASK
TX_THREAD  *UPDATE_State_Check_Thread_Handle;
UCHAR       UPDATE_State_Check_stack[UPDATE_STATE_CHECK_STACK_SIZE];
#endif

#ifdef PWM_DEMO_TASK
TX_THREAD  *PWM_Thread_Handle;
UCHAR       PWM_stack[PWM_STACK_SIZE];
#endif

#ifdef IIC_DEMO_TASK
TX_THREAD  *IIC_Thread_Handle;
UCHAR       IIC_stack[IIC_STACK_SIZE];
#endif

#ifdef RTOS_DEMO_TASK
TX_THREAD  *RTOS_Thread_Handle;
UCHAR       RTOS_stack[RTOS_STACK_SIZE];
#endif

extern void Uart_Debug_Initialize(void);

/**
 * @brief DAM application start entry point.
 *
 * Performs platform initialisation: UART debug setup, allocates the
 * demo byte pool, creates common semaphores and starts the configured
 * demo threads.
 *
 * @retval TX_SUCCESS  All resources created successfully.
 */
UINT qcli_dam_app_start(void)
{
    UINT Result;
    UINT status;

    /* Initialise debug UART and EasyLogger */
    Uart_Debug_Initialize();

    /* Allocate the shared byte pool */
    status = txm_module_object_allocate(&byte_pool_dam, sizeof(TX_BYTE_POOL));
    if (status != TX_SUCCESS) {
        log_e("[PAL] FAIL alloc pool:%d", status);
        return status;
    }

    status = tx_byte_pool_create(byte_pool_dam, "byte pool dam",
                                 free_memory_dam, DAM_BYTE_POOL_SIZE);
    if (status != TX_SUCCESS) {
        log_e("[PAL] FAIL create pool:%d", status);
        return status;
    }

    log_i("demo app starts");
#if 0
    /* ---- Create semaphores ---- */
    log_d("[PAL] 4:sem1 alloc");
    status = txm_module_object_allocate(&Net_Active_semaphore,
                                        sizeof(TX_SEMAPHORE));
    log_d("[PAL] 5:sem1 alloc=%d", status);
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate Net_Active_semaphore: %d", status);
        return status;
    }
    status = tx_semaphore_create(Net_Active_semaphore,
                                 "Net_Active_semaphore_name", 0);
    log_d("[PAL] 6:sem1 create=%d", status);
    if (status != TX_SUCCESS) {
        log_e("Failed to create Net_Active_semaphore: %d", status);
        return status;
    }

    log_d("[PAL] 7:sem2");
    status = txm_module_object_allocate(&GET_Tracking_location_semaphore,
                                        sizeof(TX_SEMAPHORE));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate GET_Tracking_location_semaphore: %d", status);
        return status;
    }
    status = tx_semaphore_create(GET_Tracking_location_semaphore,
                                 "GET_Tracking_location_semaphore_name", 0);
    if (status != TX_SUCCESS) {
        log_e("Failed to create GET_Tracking_location_semaphore: %d", status);
        return status;
    }

    log_d("[PAL] 8:sem3");
    status = txm_module_object_allocate(&Update_semaphore,
                                        sizeof(TX_SEMAPHORE));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate Update_semaphore: %d", status);
        return status;
    }
    status = tx_semaphore_create(Update_semaphore,
                                 "Update_semaphore_name", 0);
    if (status != TX_SUCCESS) {
        log_e("Failed to create Update_semaphore: %d", status);
        return status;
    }
#endif
    /* ---- Create threads ---- */

#ifdef HELLO_WORLD_TASK

    status = txm_module_object_allocate(&HELLO_WORLD_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate HELLO_WORLD_Thread: %d", status);
    } else {
        Result = tx_thread_create(HELLO_WORLD_Thread_Handle,
                                  "HELLO WORLD Thread", Hello_world_Thread, 0,
                                  Hello_world_stack, HELLO_WORLD_STACK_SIZE,
                                  HELLO_WORLD_PRIORITY, HELLO_WORLD_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start HELLO_WORLD thread: %d", Result);
    }
#endif

#ifdef SOCKET_DEMO_TASK
    status = txm_module_object_allocate(&Socket_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate Socket_Thread: %d", status);
    } else {
        Result = tx_thread_create(Socket_Thread_Handle,
                                  "SOCKET Thread", Socket_Thread, 0,
                                  Socket_stack, SOCKET_STACK_SIZE,
                                  SOCKET_PRIORITY, SOCKET_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start SOCKET_DEMO thread: %d", Result);
    }
#endif

#ifdef GPIO_DEMO_TASK
    status = txm_module_object_allocate(&GPIO_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate GPIO_Thread: %d", status);
    } else {
        Result = tx_thread_create(GPIO_Thread_Handle,
                                  "GPIO Thread", GPIO_Thread, 0,
                                  GPIO_stack, GPIO_STACK_SIZE,
                                  GPIO_PRIORITY, GPIO_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start GPIO_DEMO thread: %d", Result);
    }
#endif

#ifdef MQTT_DEMO_TASK
    status = txm_module_object_allocate(&MQTT_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate MQTT_Thread: %d", status);
    } else {
        Result = tx_thread_create(MQTT_Thread_Handle,
                                  "MQTT Thread", MQTT_Thread, 0,
                                  MQTT_stack, MQTT_STACK_SIZE,
                                  MQTT_PRIORITY, MQTT_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start MQTT_DEMO thread: %d", Result);
    }
#endif

#ifdef AT_DEMO_TASK
    status = txm_module_object_allocate(&AT_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate AT_Thread: %d", status);
    } else {
        Result = tx_thread_create(AT_Thread_Handle,
                                  "AT Thread", AT_Thread, 0,
                                  AT_stack, AT_STACK_SIZE,
                                  AT_PRIORITY, AT_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start AT_DEMO thread: %d", Result);
    }
#endif

#ifdef ADC_DEMO_TASK
    status = txm_module_object_allocate(&ADC_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate ADC_Thread: %d", status);
    } else {
        Result = tx_thread_create(ADC_Thread_Handle,
                                  "ADC Thread", ADC_Thread, 0,
                                  ADC_stack, ADC_STACK_SIZE,
                                  ADC_PRIORITY, ADC_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start ADC_DEMO thread: %d", Result);
    }
#endif

#ifdef FS_DEMO_TASK
    status = txm_module_object_allocate(&FS_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate FS_Thread: %d", status);
    } else {
        Result = tx_thread_create(FS_Thread_Handle,
                                  "FS Thread", FS_Thread, 0,
                                  FS_stack, FS_STACK_SIZE,
                                  FS_PRIORITY, FS_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start FS_DEMO thread: %d", Result);
    }
#endif

#ifdef HTTP_DEMO_TASK
    status = txm_module_object_allocate(&HTTP_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate HTTP_Thread: %d", status);
    } else {
        Result = tx_thread_create(HTTP_Thread_Handle,
                                  "HTTP Thread", HTTP_Thread, 0,
                                  HTTP_stack, HTTP_STACK_SIZE,
                                  HTTP_PRIORITY, HTTP_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start HTTP_DEMO thread: %d", Result);
    }
#endif

#ifdef GPS_DEMO_TASK
    status = txm_module_object_allocate(&GPS_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate GPS_Thread: %d", status);
    } else {
        Result = tx_thread_create(GPS_Thread_Handle,
                                  "GPS Thread", GPS_Thread, 0,
                                  GPS_stack, GPS_STACK_SIZE,
                                  GPS_PRIORITY, GPS_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start GPS_DEMO thread: %d", Result);
    }
#endif

#ifdef UPDATE_DEMO_TASK
    status = txm_module_object_allocate(&UPDATE_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate UPDATE_Thread: %d", status);
    } else {
        Result = tx_thread_create(UPDATE_Thread_Handle,
                                  "UPDATE Thread", UPDATE_Thread, 0,
                                  UPDATE_stack, UPDATE_STACK_SIZE,
                                  UPDATE_PRIORITY, UPDATE_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start UPDATE_DEMO thread: %d", Result);
    }
#endif

#ifdef UPDATE_STATE_CHECK_DEMO_TASK
    status = txm_module_object_allocate(&UPDATE_State_Check_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate UPDATE_State_Check_Thread: %d", status);
    } else {
        Result = tx_thread_create(UPDATE_State_Check_Thread_Handle,
                                  "UPDATE_State_Check Thread",
                                  UPDATE_State_Check_Thread, 0,
                                  UPDATE_State_Check_stack,
                                  UPDATE_STATE_CHECK_STACK_SIZE,
                                  UPDATE_STATE_CHECK_PRIORITY,
                                  UPDATE_STATE_CHECK_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start UPDATE_STATE_CHECK_DEMO thread: %d", Result);
    }
#endif

#ifdef PWM_DEMO_TASK
    status = txm_module_object_allocate(&PWM_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate PWM_Thread: %d", status);
    } else {
        Result = tx_thread_create(PWM_Thread_Handle,
                                  "PWM Thread", PWM_Thread, 0,
                                  PWM_stack, PWM_STACK_SIZE,
                                  PWM_PRIORITY, PWM_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start PWM_DEMO thread: %d", Result);
    }
#endif

#ifdef IIC_DEMO_TASK
    status = txm_module_object_allocate(&IIC_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate IIC_Thread: %d", status);
    } else {
        Result = tx_thread_create(IIC_Thread_Handle,
                                  "IIC Thread", IIC_Thread, 0,
                                  IIC_stack, IIC_STACK_SIZE,
                                  IIC_PRIORITY, IIC_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start IIC_DEMO thread: %d", Result);
    }
#endif

#ifdef RTOS_DEMO_TASK
    status = txm_module_object_allocate(&RTOS_Thread_Handle,
                                        sizeof(TX_THREAD));
    if (status != TX_SUCCESS) {
        log_e("Failed to allocate RTOS_Thread: %d", status);
    } else {
        Result = tx_thread_create(RTOS_Thread_Handle,
                                  "RTOS Thread", RTOS_Thread, 0,
                                  RTOS_stack, RTOS_STACK_SIZE,
                                  RTOS_PRIORITY, RTOS_PRIORITY,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        if (Result != TX_SUCCESS)
            log_e("Failed to start RTOS_DEMO thread: %d", Result);
    }
#endif

    return TX_SUCCESS;
}
