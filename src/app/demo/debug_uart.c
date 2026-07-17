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
#include "debug_port_cfg.h"

#define DEBUG_MAX_SEND_BUFFER_SIZE 1024

/** @brief Mutex protecting Debug_Printf static buffers (shared by USB/UART). */
TX_MUTEX *Debug_Printf_mutex;

#ifdef DEBUG_USE_UART
TX_THREAD			* UartRx_Thread_Handle;
UCHAR			    UartRx_Thread_stack[1024*5];

TX_SEMAPHORE *UartRx_semaphore;

#define UartRxSize 1024
char UartRxBuf[UartRxSize]={0};
volatile uint32_t UartRx_num_bytes;

qapi_UART_Handle_t handle;

/** @brief Signalled when UART TX DMA completes. */
TX_SEMAPHORE *UartTx_semaphore;
#endif /* DEBUG_USE_UART */

#ifdef DEBUG_USE_USB
static bool usb_initialized = false;
#endif

#define 		EASYLOG_STACK_SIZE (2 * 1024)
extern void Easylog_async_output_task(void *arg);
TX_THREAD		*Easylog_Thread_Handle;
UCHAR			Easylog_stack[EASYLOG_STACK_SIZE];
extern TX_SEMAPHORE *elog_dma_lockHandle;

void UART_Write(uint32_t Length, const char *Buffer);
void Debug_Printf(char* format, ...);

#ifdef DEBUG_USE_UART
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
 * Signals the UART TX semaphore when a DMA transfer completes.
 * Also signals the EasyLogger DMA lock semaphore so elog_port_output
 * can unblock after its UART_Write completes.
 *
 * @param[in] num_bytes Number of bytes transmitted.
 * @param[in] cb_data Pointer to callback-specific data (unused).
 */
static void dam_cli_tx_cb(uint32_t num_bytes, void *cb_data)
{
    if (elog_dma_lockHandle)
        tx_semaphore_put(elog_dma_lockHandle);
    if (UartTx_semaphore)
        tx_semaphore_put(UartTx_semaphore);
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
#endif /* DEBUG_USE_UART */

/**
 * @brief Initialize debug output port and start EasyLogger.
 *
 * When DEBUG_USE_UART is defined: opens UART, creates RX thread,
 * configures DMA-based TX. Elog async task waits on DMA completion.
 *
 * When DEBUG_USE_USB is defined: opens USB virtual serial (blocking I/O).
 * Elog async task still runs — reads ring buffer, writes via USB (blocking).
 * No DMA callback needed since qapi_USB_Write is synchronous.
 */
void Uart_Debug_Initialize(void)
{
   int Result;
   UINT status;

   /* Allocate and create mutex for Debug_Printf thread safety */
   status = txm_module_object_allocate(&Debug_Printf_mutex, sizeof(TX_MUTEX));
   if(status != TX_SUCCESS)
   {
     return;
   }
   status = tx_mutex_create(Debug_Printf_mutex, "Debug_Printf_mutex", TX_NO_INHERIT);
   if(status != TX_SUCCESS)
   {
     return;
   }

#ifdef DEBUG_USE_UART
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
   qapi_UART_Power_On(handle);
   qapi_UART_Receive(handle, UartRxBuf, UartRxSize, 0);

   /* Allocate and create UART TX completion semaphore */
   status = txm_module_object_allocate(&UartTx_semaphore, sizeof(TX_SEMAPHORE));
   if(status != TX_SUCCESS)
   {
     return;
   }
   status = tx_semaphore_create(UartTx_semaphore, "UartTx_semaphore", 0);
   if(status != TX_SUCCESS)
   {
     return;
   }

   /* definition and creation of UartRx_semaphore */
   status = txm_module_object_allocate(&UartRx_semaphore, sizeof(TX_SEMAPHORE));
   if(status != TX_SUCCESS)
   {
     return;
   }
   status = tx_semaphore_create(UartRx_semaphore,"UartRx_semaphore_name", 0);
   if(status != TX_SUCCESS)
   {
     return;
   }

   /* definition and creation of UartRx_Thread */
   status = txm_module_object_allocate(&UartRx_Thread_Handle, sizeof(TX_THREAD));
   if(status != TX_SUCCESS)
   {
     /* non-fatal, RX thread is optional */
   }
   else
   {
     Result = tx_thread_create(UartRx_Thread_Handle, "UartRx Thread",
                               UartRx_Thread, 0, UartRx_Thread_stack,
                               sizeof(UartRx_Thread_stack),
                               148, 148, TX_NO_TIME_SLICE, TX_AUTO_START);
     if(Result != TX_SUCCESS)
     {
         /* non-fatal */
     }
   }
#endif /* DEBUG_USE_UART */

#ifdef DEBUG_USE_USB
   if(qapi_USB_Open() == QAPI_OK)
   {
     usb_initialized = true;
   }
   /* USB is blocking, no semaphore or RX thread needed */
#endif

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

       /* Async mode: log_i() writes to ring buffer, async task handles output.
        * UART: async task waits on DMA completion semaphore after UART_Write.
        * USB:  async task calls qapi_USB_Write (blocking, no DMA callback). */
       elog_async_enabled(true);

       #ifdef ELOG_BUF_OUTPUT_ENABLE
       elog_buf_enabled( true );
       #endif
       /* start EasyLogger */
       elog_start();
   }

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
   /* definition and creation of Easylog_async_output_task */
   status = txm_module_object_allocate(&Easylog_Thread_Handle, sizeof(TX_THREAD));
   if(status != TX_SUCCESS)
   {
       return;
   }
   Result = tx_thread_create(Easylog_Thread_Handle, "Easylog Thread", Easylog_async_output_task, 152, Easylog_stack,
                                 EASYLOG_STACK_SIZE, 170, 170, TX_NO_TIME_SLICE, TX_AUTO_START);
   if(Result != TX_SUCCESS)
   {
       /* non-fatal */
   }
#endif /* ELOG_ASYNC_OUTPUT_ENABLE */
}

/**
 * @brief Write raw bytes to the selected debug output port.
 *
 * UART mode: sends via async DMA (`qapi_UART_Transmit`), caller must
 *            wait on completion semaphore for synchronous behavior.
 * USB mode:  sends via blocking `qapi_USB_Write`, returns after data
 *            is written (no completion callback needed).
 *
 * @param[in] Length Number of bytes to write.
 * @param[in] Buffer Pointer to data to transmit.
 */
void UART_Write(uint32_t Length, const char *Buffer)
{
   if((Length) && (Buffer))
   {
#ifdef DEBUG_USE_UART
       qapi_UART_Transmit(handle, Buffer, Length, (void*)Buffer);
#endif
#ifdef DEBUG_USE_USB
       if(usb_initialized)
           qapi_USB_Write((void*)Buffer, (uint16_t)Length);
#endif
   }
}

/**
 * @brief Formatted printf-like output to the selected debug port.
 *
 * Formats the supplied arguments into a temporary buffer and writes the
 * result via UART (DMA + semaphore wait) or USB (blocking write).
 *
 * @param[in] format printf-style format string.
 * @param[in] ... format arguments.
 */
void Debug_Printf(char* format, ...)
{
    static char str_tmp[DEBUG_MAX_SEND_BUFFER_SIZE] = {0};
    int Length=0;
    va_list vArgList;
    tx_mutex_get(Debug_Printf_mutex, TX_WAIT_FOREVER);
    va_start (vArgList, format);
    Length=vsnprintf(str_tmp, DEBUG_MAX_SEND_BUFFER_SIZE, format, vArgList);
    va_end(vArgList);
    UART_Write(Length, str_tmp);
#ifdef DEBUG_USE_UART
    /* UART uses async DMA — wait for TX completion callback */
    tx_semaphore_get(UartTx_semaphore, TX_WAIT_FOREVER);
#endif
    /* USB: qapi_USB_Write already returned after blocking write */
    tx_mutex_put(Debug_Printf_mutex);
}

