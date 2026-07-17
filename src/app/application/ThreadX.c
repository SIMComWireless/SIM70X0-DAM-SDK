/**
 * @file ThreadX.c
 * @brief Application thread definitions and demo task implementations.
 *
 * Contains demo thread entry functions used by the platform to run
 * example workloads (network, GPIO, MQTT, etc.).  Each function is a
 * ThreadX entry routine that accepts a @c void* parameter.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)
#define LOG_TAG    "Thread_demo"
#endif

#include "ThreadX.h"
#include "txm_module.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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
#include "qapi_ns_utils.h"
#include "qapi_ssl.h"
#include "qapi_httpc.h"
#include "qflog_utils.h"

#include "../demo/net_work_demo.h"
#include "../demo/socket_demo.h"
#include "../demo/GPIO_demo.h"
#include "../demo/mqtt_demo.h"
#include "../demo/at_demo.h"
#include "../demo/adc_demo.h"
#include "../demo/fs_demo.h"
#include "../demo/http_demo.h"
#include "../demo/gps_demo.h"
#include "../demo/update_demo.h"
#include "../demo/pwm_demo.h"
#include "../demo/iic_demo.h"
#include "../demo/rtos_demo.h"

#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern TX_SEMAPHORE *Net_Active_semaphore;
extern TX_SEMAPHORE *Update_semaphore;

/** @brief Scratch buffer used by multiple demo threads. */
uint8 Buf[50];

/** @brief Size of the OTA update verification buffer (30 KiB). */
#define UPDATE_BUF_SIZE (1024 * 30)

/** @brief Buffer for OTA update file CRC verification. */
char update_buf[UPDATE_BUF_SIZE] = {0};

/*-------------------------------------------------------------------------
 * Thread Entry Functions
 *-----------------------------------------------------------------------*/

/**
 * @brief Hello-world demo thread entry.
 *
 * Periodically increments a counter and logs a greeting message.
 * Useful for verifying that the DAM task scheduler is running.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void Hello_world_Thread(void *Param)
{
    static uint32_t count = 0;
    (void)Param;
    while (1)
    {
        log_i("Hello world, SIMCom! Count:%d", count++);
        //qapi_Timer_Sleep(100, QAPI_TIMER_UNIT_MSEC, true);
		tx_thread_sleep(100);
    }
}

/**
 * @brief Socket demo thread entry.
 *
 * Brings up the PDN, waits for the network to become active, then
 * runs a TCP send/receive followed by a UDP send/receive in a loop.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void Socket_Thread(void *Param)
{
    char *temp_str = NULL;
    (void)Param;

    tx_thread_sleep(300);
    temp_str = "cmnet";
    DAM_NW_dss_init(temp_str);
    tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);

    while (1)
    {
        log_i("TCP_SOCKET");
        socket_demo_TCP((char *)"112.74.93.163", 5600,
                         (char *)"hello world TCP Server");
        qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);
        socket_demo_UDP((char *)"112.74.93.163", 1124,
                         (char *)"hello world UDP Server");
        tx_thread_sleep(1000);
    }
}

/**
 * @brief GPIO demo thread entry.
 *
 * Toggles an output pin (GPIO6) every second and registers an
 * interrupt on GPIO7.  After 10 toggles the interrupt is
 * deregistered to demonstrate the cleanup path.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void GPIO_Thread(void *Param)
{
    static uint8_t level = 0;
    int i = 0;
    (void)Param;

    log_i("GPIO_Thread started");
    gpio_init(PIN67_GPIO6, GPIO_OUTPUT_DIR, GPIO_HIGH_LEVEL);
    gpio_interrupt_register(PIN68_GPIO7);

    while (1)
    {
        gpio_write_pin(PIN67_GPIO6, level);
        level = !level;
        qapi_Timer_Sleep(1000, QAPI_TIMER_UNIT_MSEC, true);
        log_i("GPIO Toggle");
        i++;
        if (i == 10)
        {
            gpio_interrupt_deregister(PIN68_GPIO7);
            log_i("Disable INT");
        }
    }
}

/**
 * @brief MQTT demo thread entry.
 *
 * Brings up the network, connects to an MQTT broker, subscribes
 * to a topic and publishes incrementing messages in a loop.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void MQTT_Thread(void *Param)
{
    char *temp_str = NULL;
    unsigned int count = 0;
    unsigned char test_string[32] = {0};
    qapi_Status_t ret;
    (void)Param;

    temp_str = "cmnet";
    tx_thread_sleep(300);
    DAM_NW_dss_init(temp_str);
    tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
    mqtt_init();
    mqtt_config_set("simcom", 60, 1, NULL, NULL);
    ret = mqtt_connect("mq.tongxinmao.com", 18830);
    if (ret == 0)
    {
        mqtt_subscribe("/simcom/test", 1);
        while (!ret)
        {
            snprintf((char *)test_string, sizeof(test_string),
                     "count:%u", count);
            ret = mqtt_publish("/simcom/test", 0, 0, test_string);
            count++;
            tx_thread_sleep(1000);
        }
        log_e("MQTT publish failed, disconnecting");
        mqtt_disconnect();
        mqtt_deinit();
    }
    else
    {
        log_e("mqtt_connect fail");
        mqtt_deinit();
    }
}

/**
 * @brief AT command demo thread entry.
 *
 * Periodically sends an AT command on the visual AT channel.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void AT_Thread(void *Param)
{
    (void)Param;
	static uint32_t count = 0;
    while (1)
    {
        log_i("AT_DEMO Count:%d", count++);
        atcmd("AT+CGREG?");
        tx_thread_sleep(10);
    }
}

/**
 * @brief ADC demo thread entry.
 *
 * Opens the ADC, reads channel 6 (0-1.8 V range), closes the ADC,
 * then sleeps for 3 seconds before repeating.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void ADC_Thread(void *Param)
{
    (void)Param;

    while (1)
    {
        log_i("ADC_DEMO!!");
        adc_demo_Open();
        adc_demo_Read_Channel(6);
        adc_demo_Close();
        qapi_Timer_Sleep(3, QAPI_TIMER_UNIT_SEC, true);
    }
}

/** @brief Directory used by the FS demo. */
#define FS_DEMO_DIR   "customer/configuration"

/** @brief File path used by the FS demo. */
#define FS_DEMO_FILE  "customer/configuration/fs.txt"

/**
 * @brief File-system demo thread entry.
 *
 * Demonstrates directory creation, file write/read, stat, and
 * deletion in an infinite loop.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void FS_Thread(void *Param)
{
    int Fd_ptr;
    (void)Param;

    while (1)
    {
        log_i("FS_DEMO!!");
        if ((fs_demo_Mk_Dir(FS_DEMO_DIR, QAPI_FS_S_IXUSR_E) == QAPI_ERR_EXISTS) ||
            (fs_demo_Mk_Dir(FS_DEMO_DIR, QAPI_FS_S_IXUSR_E) == QAPI_OK))
        {
            /* Write data */
            if (fs_demo_Open(FS_DEMO_FILE,
                             QAPI_FS_O_CREAT_E | QAPI_FS_O_RDWR_E,
                             &Fd_ptr) == QAPI_OK)
            {
                fs_demo_Write(Fd_ptr, (uint8 *)"0123456789", 10);
                fs_demo_Close(Fd_ptr);
            }

            /* Get file info */
            fs_demo_Stat(FS_DEMO_FILE);

            /* Read back */
            if (fs_demo_Open(FS_DEMO_FILE,
                             QAPI_FS_O_CREAT_E | QAPI_FS_O_RDWR_E,
                             &Fd_ptr) == QAPI_OK)
            {
                fs_demo_seek(Fd_ptr, 3);
                fs_demo_Read(Fd_ptr, Buf, 4);
                fs_demo_Close(Fd_ptr);
            }

            /* Clean up */
            qapi_FS_Unlink(FS_DEMO_FILE);
            qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);
        }
    }
}

/**
 * @brief HTTP demo thread entry.
 *
 * Brings up the network, connects to an HTTP server, then
 * periodically performs GET requests and stores the response
 * to the file system.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void HTTP_Thread(void *Param)
{
    char *temp_str = NULL;
    uint32_t ret;
    (void)Param;

    tx_thread_sleep(300);
    temp_str = "cmnet";
    DAM_NW_dss_init(temp_str);
    tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
    http_demo_init();
    ret = HTTP_Connect("112.74.93.163", 5050);
    if (ret)
    {
        log_e("HTTP connect failed!");
        return;
    }

    while (1)
    {
        log_i("HTTP_demo");
        tx_thread_sleep(500);
        HTTP_Request(QAPI_NET_HTTP_CLIENT_GET_E,
                     (uint8_t *)"https://www.baidu.com/");
    }
}

/**
 * @brief GPS demo thread entry.
 *
 * Initialises the GNSS client and performs single-shot tracking
 * requests in a loop.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void GPS_Thread(void *Param)
{
    (void)Param;

    tx_thread_sleep(100);
    gps_demo_init();

    while (1)
    {
        log_i("GPS_demo");
        tracking_test_singleshot(500, 0.4);
        tx_thread_sleep(100);
    }
}

/**
 * @brief Application OTA update thread entry.
 *
 * Brings up the network, downloads an update image via AT+HTTPTOFS,
 * verifies its CRC, and optionally triggers the update.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void UPDATE_Thread(void *Param)
{
    char *temp_str = NULL;
    (void)Param;

    tx_thread_sleep(300);
    temp_str = "cmnet";
    DAM_NW_dss_init(temp_str);
    tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);

    while (1)
    {
        tx_thread_sleep(500);
        log_i("UPDATE_demo");
        /* Download the update image (modify URL as needed) */
        atcmd("AT+HTTPTOFS=http://117.131.85.139:6002/myweb/image/"
              "cust_app.bin,/custapp/cust_app.update");
        tx_semaphore_get(Update_semaphore, TX_WAIT_FOREVER);

        if (file_check("/custapp/cust_app.update",
                       (uint8 *)update_buf,
                       UPDATE_BUF_SIZE) == DAM_STATUS_SUCCESS)
        {
            log_i("Start to Update...");
            /* qapi_DAM_Update_App(); */
        }
    }
}

/**
 * @brief Update-state check thread entry.
 *
 * Periodically checks whether the last OTA update completed
 * successfully.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void UPDATE_State_Check_Thread(void *Param)
{
    (void)Param;

    while (1)
    {
        tx_thread_sleep(500);
        log_i("UPDATE_State_Check_demo");
        update_state_check();
    }
}

/**
 * @brief PWM demo thread entry.
 *
 * Initialises the PWM peripheral and logs a heartbeat message
 * every 500 ms.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void PWM_Thread(void *Param)
{
    (void)Param;

    dam_pwm_init();
    while (1)
    {
        tx_thread_sleep(500);
        log_i("PWM Running");
    }
}

/**
 * @brief I2C (IIC) demo thread entry.
 *
 * Writes a pattern to an EEPROM at address 0x50, then reads
 * it back every second and logs the contents.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void IIC_Thread(void *Param)
{
    uint8_t reg[2] = {0x00, 0x00};
    uint8_t read_buf[16] = {0};
    uint8_t write_buf[16] = {0};
    uint16_t i;
    uint8_t slave_address = 0x50;
    (void)Param;

    iic_init();
    memset(write_buf, 30, sizeof(write_buf));
    i2c_write(slave_address, reg, sizeof(reg),
              write_buf, sizeof(write_buf));

    while (1)
    {
        log_i("IIC Demo Running");
        memset(read_buf, 0, sizeof(read_buf));
        i2c_read(slave_address, reg, sizeof(reg),
                 read_buf, sizeof(read_buf));

        for (i = 0; i < sizeof(read_buf); i++)
            log_i("read_buf[%d]:%d", i, read_buf[i]);

        qapi_Timer_Sleep(1000, QAPI_TIMER_UNIT_MSEC, true);
    }
}

/**
 * @brief RTOS demo thread entry.
 *
 * Delegates to rtos_demo_entry() which creates several ThreadX
 * threads, queues, semaphores, mutexes and event flags as a
 * reference example.
 *
 * @param[in] Param  Thread entry parameter (unused).
 */
void RTOS_Thread(void *Param)
{
    (void)Param;

    rtos_demo_entry();
    while (1)
    {
        tx_thread_sleep(500);
    }
}
