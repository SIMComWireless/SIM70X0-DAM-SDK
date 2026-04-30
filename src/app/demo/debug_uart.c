/**
 * @file debug_uart.c
 * @brief UART/USB debug helper and EasyLogger integration.
 *
 * Implements UART initialization, RX/TX callbacks and convenience
 * print functions used by the demo applications. The module also
 * initializes and starts the EasyLogger async output task when enabled.
 *
 * Notes:
 * - QAPI_UART_PORT_001_E -> module UART2
 * - QAPI_UART_PORT_002_E -> module UART3
 * - QAPI_UART_PORT_003_E -> module UART1
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "qapi_usb.h"
#include "qapi_usb_types.h"
#include "qapi_uart.h"
#include "qapi_timer.h"
#include "txm_module.h"

#include "../Easylogger/elog.h"

UCHAR			   LOG_stack[1024]; 

#define DEBUG_MAX_SEND_BUFFER_SIZE 1024

TX_THREAD			* UartRx_Thread_Handle;		 
UCHAR			    UartRx_Thread_stack[1024*5]; 

TX_SEMAPHORE *UartRx_semaphore;

#define UartRxSize 1024
char UartRxBuf[UartRxSize]={0};
volatile UartRx_num_bytes;

qapi_UART_Handle_t handle;

void Debug_Printf(char* format, ...);
void Debug_USB_Printf(char* format, ...);

#define 		EASYLOG_STACK_SIZE 2*1024
extern void Easylog_async_output_task(void *arg);
TX_THREAD		*Easylog_Thread_Handle;
UCHAR			Easylog_stack[EASYLOG_STACK_SIZE];
extern TX_SEMAPHORE *elog_dma_lockHandle;

void UART_Write(uint32_t Length, const char *Buffer);
void Debug_USB_Printf(char* format, ...);
void Debug_Printf(char* format, ...);

/**
 * @brief UART RX callback invoked by QAPI when data arrives.
 *
 * This callback stores the received byte count and releases the UART
 * RX semaphore so the `UartRx_Thread` can process the received data.
 *
 * @param[in] num_bytes Number of bytes received by the UART driver.
 * @param[in] cb_data Pointer to callback-specific data (unused).
 */
static void dam_cli_rx_cb(uint32_t num_bytes, void *cb_data)
{
    UartRx_num_bytes=num_bytes;
    tx_semaphore_put(UartRx_semaphore);	
}

/**
 * @brief UART TX completion callback.
 *
 * Released the EasyLogger DMA lock semaphore when a TX completes.
 *
 * @param[in] num_bytes Number of bytes transmitted.
 * @param[in] cb_data Pointer to callback-specific data (unused).
 */
static void dam_cli_tx_cb(uint32_t num_bytes, void *cb_data)
{
    tx_semaphore_put(elog_dma_lockHandle);
}

/**
 * @brief UART RX processing thread.
 *
 * Waits on `UartRx_semaphore` and, when signaled, calls the QAPI
 * UART receive API to fetch incoming data and logs the number of bytes
 * received.
 *
 * @param[in] Param Thread entry parameter (unused).
 */
void UartRx_Thread(void *Param)
{
	
	while (1)
		{
      tx_semaphore_get(UartRx_semaphore, TX_WAIT_FOREVER);
      if(QAPI_OK == qapi_UART_Receive(handle, UartRxBuf, UartRxSize, 0))
		  {
        log_i("Uart RX->UartRx_num_bytes:%d",UartRx_num_bytes);
		  }
		}
  
}

/**
 * @brief Initialize UART port and start EasyLogger (if enabled).
 *
 * Opens the configured UART port, creates the RX semaphore and thread,
 * configures EasyLogger formatting and starts the async logging task
 * when ELOG_ASYNC_OUTPUT_ENABLE is defined.
 */
void Uart_Debug_Initialize(void)
{
   int Result;
   UINT status;
	
   qapi_UART_Open_Config_t open_properties;
   open_properties.parity_Mode = QAPI_UART_NO_PARITY_E;
   open_properties.num_Stop_Bits= QAPI_UART_1_0_STOP_BITS_E;
   open_properties.baud_Rate   = 115200;
   open_properties.bits_Per_Char= QAPI_UART_8_BITS_PER_CHAR_E;
   open_properties.rx_CB_ISR = dam_cli_rx_cb;
   open_properties.tx_CB_ISR = dam_cli_tx_cb;
   open_properties.enable_Flow_Ctrl = false;
   open_properties.enable_Loopback= false;

   if( qapi_UART_Open(&handle, QAPI_UART_PORT_003_E, &open_properties) != QAPI_OK)
   {
      return;
   }
   else
   {
    qapi_UART_Power_On(handle);
    qapi_UART_Receive(handle,  UartRxBuf, UartRxSize, 0);

      /* definition and creation of UartRx_semaphore */	
    txm_module_object_allocate(&UartRx_semaphore, sizeof(TX_SEMAPHORE)); 
    status = tx_semaphore_create(UartRx_semaphore,"UartRx_semaphore_name", 0);
    if(status != TX_SUCCESS)
    {
      log_e("Failed to start UartRx_semaphore");
    }

    /* definition and creation of UartRx_Thread */
    txm_module_object_allocate(&UartRx_Thread_Handle, sizeof(TX_THREAD));
    Result = tx_thread_create(UartRx_Thread_Handle, "UartRx Thread", UartRx_Thread, 152, UartRx_Thread_stack,
                                    sizeof(LOG_stack), 150, 150, TX_NO_TIME_SLICE, TX_AUTO_START);
    if(Result != TX_SUCCESS)
      {
          Debug_Printf("Failed to start UartRx thread\r\n");
      }
    }

   if (elog_init() == ELOG_NO_ERR)
	{
		/* set enabled format */
		elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
		elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL );
		elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
		elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_TAG | ELOG_FMT_TIME);
		elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
		elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));

		elog_set_text_color_enabled( true );
    elog_async_enabled(true);
		
		#ifdef ELOG_BUF_OUTPUT_ENABLE
		elog_buf_enabled( true );
		#endif
		/* start EasyLogger */
		elog_start();
	}

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
	/* definition and creation of Easylog_async_output_task */
	txm_module_object_allocate(&Easylog_Thread_Handle, sizeof(TX_THREAD));
    Result = tx_thread_create(Easylog_Thread_Handle, "Easylog Thread", Easylog_async_output_task, 152, Easylog_stack,
                                  EASYLOG_STACK_SIZE, 161, 161, TX_NO_TIME_SLICE, TX_AUTO_START);
	if(Result != TX_SUCCESS)
    {
        Debug_Printf("Failed to start Easylog Thread");
    }
#endif

}

/**
 * @brief Write raw bytes to the UART port.
 *
 * Sends `Length` bytes from `Buffer` using `qapi_UART_Transmit`.
 * The UART must be opened prior to calling this function.
 *
 * @param[in] Length Number of bytes to write.
 * @param[in] Buffer Pointer to data to transmit.
 */
void UART_Write(uint32_t Length, const char *Buffer)
{
   if((Length) && (Buffer))
   {
       qapi_UART_Transmit(handle,Buffer, Length, (void*)Buffer);
   }
}

/**
 * @brief Formatted printf-like output over UART.
 *
 * Formats the supplied arguments into a temporary buffer and writes the
 * result to UART via `UART_Write`. The UART must be initialized first.
 *
 * @param[in] format printf-style format string.
 * @param[in] ... format arguments.
 */
void Debug_Printf(char* format, ...)
{
    static char str_tmp[DEBUG_MAX_SEND_BUFFER_SIZE] = {0};
    int Length=0;
    va_list vArgList; 
    va_start (vArgList, format); 
    Length=vsnprintf(str_tmp, DEBUG_MAX_SEND_BUFFER_SIZE, format, vArgList);
    va_end(vArgList);
    UART_Write(Length,str_tmp);
    //qapi_Timer_Sleep(2,QAPI_TIMER_UNIT_MSEC,true);
}

/**
 * @brief Formatted printf-like output over USB.
 *
 * Formats the supplied arguments into a temporary buffer and writes the
 * result to USB using `qapi_USB_Write`. Attempts to open USB on first use.
 *
 * @param[in] format printf-style format string.
 * @param[in] ... format arguments.
 */
void Debug_USB_Printf(char* format, ...)
{
    static char str_tmp[DEBUG_MAX_SEND_BUFFER_SIZE] = {0};
    int Length=0;
    va_list vArgList; 
    static qapi_USB_Status_t usb_status=-1;
    if(usb_status!= QAPI_OK){
      usb_status = qapi_USB_Open();
    }
    va_start (vArgList, format); 
    Length=vsnprintf(str_tmp, DEBUG_MAX_SEND_BUFFER_SIZE, format, vArgList);
    va_end(vArgList);
    qapi_USB_Write(str_tmp,Length);
    qapi_Timer_Sleep(2,QAPI_TIMER_UNIT_MSEC,true);
}

