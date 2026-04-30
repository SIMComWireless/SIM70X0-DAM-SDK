/**
 * @file ThreadX.c
 * @brief Application thread definitions and demo task implementations.
 *
 * Contains demo thread entry functions used by the platform to run
 * example workloads (network, GPIO, MQTT, etc.). Each function is a
 * ThreadX entry routine that accepts a `void *` parameter.
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
#include <stdlib.h>
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

#if !defined(LOG_TAG)    
	#define LOG_TAG    "Thread"
#endif

#include "../Easylogger/elog.h"

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern TX_SEMAPHORE *Net_Active_semaphore;
extern TX_SEMAPHORE *Update_semaphore;
uint8 Buf[50];
#define UPDATE_BUF_SIZE 1024*30
char update_buf[UPDATE_BUF_SIZE] = {0};

/**
 * @brief Hello world thread entry.
 *
 * Periodically logs a "Hello world" message to the debug console.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
extern void UART_Write(uint32_t Length, const char *Buffer);
void Hello_world_Thread(void *Param)
{

	while (1)
	{
  		log_i("Hello world,SIMCom");
		qapi_Timer_Sleep(1000,QAPI_TIMER_UNIT_MSEC,true);
	}
  
}

/**
 * @brief Socket demo thread entry.
 *
 * Brings up the PDN, waits for network, and runs TCP/UDP demo transactions.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void Socket_Thread(void *Param)
{	
	char   *temp_str = NULL;
	
	tx_thread_sleep(300);
	temp_str="cmnet";
	DAM_NW_dss_init(temp_str);
	tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
	while (1)
	{
  		log_i("TCP_SOCKET");
		socket_demo_TCP((char *)"112.74.93.163",5600,(char *)"hello world TCP Server");
		qapi_Timer_Sleep(1,QAPI_TIMER_UNIT_SEC,true);
		socket_demo_UDP((char *)"112.74.93.163",1124,(char *)"hello world UDP Server");
		tx_thread_sleep(1000);
	}
}

/**
 * @brief GPIO demo thread entry.
 *
 * Toggles an output pin and demonstrates interrupt registration/deregistration.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void GPIO_Thread(void *Param)
{
	static uint8_t level = 0;
	int i=0;
	log_i("GPIO_Thread!!");

	gpio_init(PIN67_GPIO6 , GPIO_OUTPUT_DIR ,GPIO_HIGH_LEVEL);
	gpio_interrupt_register(PIN68_GPIO7);

	while(1) 
	{
		gpio_write_pin(PIN67_GPIO6 , level);
		level = !level;
		qapi_Timer_Sleep(1000,QAPI_TIMER_UNIT_MSEC,true);
		log_i("GPIO Toogle");
		i++;
		if(i == 10)
		{
			gpio_interrupt_deregister(PIN68_GPIO7);

			log_i("Disable INT");
		}
	}
  
}

/**
 * @brief MQTT demo thread entry.
 *
 * Connects to the configured MQTT broker and publishes messages periodically.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void MQTT_Thread(void * Param)
{	
	char   *temp_str = NULL;
	unsigned int count=0;
	unsigned char test_string[20]={0};
	qapi_Status_t  ret;
	temp_str="cmnet";
	tx_thread_sleep(300);
	DAM_NW_dss_init(temp_str);
	tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
	mqtt_init();
	mqtt_config_set("simcom",60,1,NULL,NULL);
	ret = mqtt_connect("mq.tongxinmao.com",18830);
	if(ret == 0)
	{
		mqtt_subscribe("/simcom/test",1);
		while (!ret)
			{
			sprintf(test_string,"count:%d" ,count);
			mqtt_publish("/simcom/test",0,0,test_string);
			count++;
			tx_thread_sleep(1000);
			}
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
 * Periodically sends AT commands to demonstrate the visual AT channel.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void AT_Thread(void * Param)
{	
	
	while (1)
	{
  		log_i("AT_DEMO!!");
		atcmd("AT+CGREG?");
		qapi_Timer_Sleep(2,QAPI_TIMER_UNIT_SEC,true);
	}
  
}

/**
 * @brief ADC demo thread entry.
 *
 * Opens ADC, reads a channel and closes it periodically.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void ADC_Thread(void * Param)
{	
	
	while (1)
	{
  		log_i("ADC_DEMO!!");
		adc_demo_Open();
		adc_demo_Read_Channel(6);//the range of ADC is 0-1.8V
		adc_demo_Close();
		qapi_Timer_Sleep(3,QAPI_TIMER_UNIT_SEC,true);
	}
  
}

#define file_dir  "customer/configuration"
#define file_path "customer/configuration/fs.txt"

/**
 * @brief File system demo thread entry.
 *
 * Demonstrates directory creation, file write/read and deletion.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void FS_Thread(void * Param)
{	
	int Fd_ptr;
	
	while (1)
	{
		log_i("FS_DEMO!!");
		//creat a  file if it does not exist
		if ((fs_demo_Mk_Dir(file_dir, QAPI_FS_S_IXUSR_E) == QAPI_ERR_EXISTS) || (fs_demo_Mk_Dir(file_dir, QAPI_FS_S_IXUSR_E) == QAPI_OK))
		{
			//write data 
			if (fs_demo_Open(file_path,QAPI_FS_O_CREAT_E | QAPI_FS_O_RDWR_E , Fd_ptr) == QAPI_OK)
			{
				fs_demo_Write(Fd_ptr, "0123456789", 10);
				fs_demo_Close(Fd_ptr);
			}	
				
			//get data information 
			fs_demo_Stat(file_path);

			if (fs_demo_Open(file_path, QAPI_FS_O_CREAT_E | QAPI_FS_O_RDWR_E, Fd_ptr) == QAPI_OK)
			{
				fs_demo_seek(Fd_ptr,3);
				fs_demo_Read(Fd_ptr, Buf, 4);
				fs_demo_Close(Fd_ptr);
			}	

			//delete file
			fs_demo_Del_Dir(file_path);
			qapi_Timer_Sleep(1,QAPI_TIMER_UNIT_SEC,true);
			

		}

		}		
  
}

/**
 * @brief HTTP demo thread entry.
 *
 * Initializes the HTTP client, connects and performs HTTP requests periodically.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void HTTP_Thread(void *Param)
{	
	char   *temp_str = NULL;
	uint32_t  ret;

	tx_thread_sleep(300);
	DAM_NW_dss_init(temp_str);
	tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
	http_demo_init();
	ret = HTTP_Connect("112.74.93.163", 5050);
	if (ret)
	{
		log_e("connect failed !!");
	}		
	while (1)
	{
		log_i("HTTP_demo");
		tx_thread_sleep(500);		
		HTTP_Request(QAPI_NET_HTTP_CLIENT_GET_E, "https://www.baidu.com/");		
	}
  
}

/**
 * @brief GPS demo thread entry.
 *
 * Starts the GPS client and performs single-shot tracking requests.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void GPS_Thread(void *Param)
{	
	uint32_t  ret;

	tx_thread_sleep(100);
	gps_demo_init();
	while (1)
	{
		log_i("GPS_demo");
		tracking_test_singleshot(500,0.4);
		tx_thread_sleep(100);
	}
  
}

/**
 * @brief Application update thread entry.
 *
 * Downloads an update image via AT+HTTPTOFS and verifies it before flashing.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void UPDATE_Thread(void *Param)
{	
	char   *temp_str = NULL;
	uint32_t  ret;

	tx_thread_sleep(300);
	DAM_NW_dss_init(temp_str);
	tx_semaphore_get(Net_Active_semaphore, TX_WAIT_FOREVER);
	while (1)
	{
		tx_thread_sleep(500);
		log_i("UPDATE_demo");
		/*you can get your file by modifying the URL*/
		atcmd("AT+HTTPTOFS=http://117.131.85.139:6002/myweb/image/cust_app.bin,/custapp/cust_app.update");	
		tx_semaphore_get(Update_semaphore, TX_WAIT_FOREVER);
		
		if (file_check("/custapp/cust_app.update",update_buf,UPDATE_BUF_SIZE) == DAM_STATUS_SUCCESS)
			{
			log_i("Star to Update...");
			//qapi_DAM_Update_App();
			}
		
	}
  
}

/**
 * @brief Update state check thread entry.
 *
 * Periodically checks whether the last update completed successfully.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void UPDATE_State_Check_Thread(void *Param)
{	
	
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
 * Initializes PWM and logs status periodically.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void PWM_Thread(void *Param)
{	
	dam_pwm_init();
	while (1)
	{
		tx_thread_sleep(500);
		log_i("PWM Running");
	}
		
  
}

/**
 * @brief I2C/IIC demo thread entry.
 *
 * Demonstrates I2C write/read transactions against a slave device.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void IIC_Thread(void *Param)
{	
	uint8_t reg[2] = {0x00,0x00};
	uint8_t read_buf[16]={0};
	uint8_t write_buf[16]={0};
	uint16_t i;
	uint8_t slave_address=0x50;

	iic_init();
	memset(write_buf,30,sizeof(write_buf));
	i2c_write(slave_address , reg , sizeof(reg), write_buf , sizeof(write_buf));

	while (1)
	{
		log_i("IIC Demo Running");

		memset(read_buf,0,sizeof(read_buf));
		i2c_read(slave_address , reg , sizeof(reg), read_buf , sizeof(read_buf));

		for(i=0;i<sizeof(read_buf);i++)
			log_i("read_buf[%d]:%d",i,read_buf[i]);

		qapi_Timer_Sleep(1000,QAPI_TIMER_UNIT_MSEC,true);
	}
		
  
}

/**
 * @brief RTOS demo thread entry.
 *
 * Invokes the `rtos_demo_entry` to run ThreadX demo examples.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void RTOS_Thread(void *Param)
{	
	rtos_demo_entry();
	while (1)
	{
		tx_thread_sleep(500);
	}
}







