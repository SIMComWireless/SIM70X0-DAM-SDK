/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include "elog.h"
#include <stdio.h>
#include "qapi_types.h"
#include "qapi.h"
#include "txm_module.h"
#include "qapi_uart.h"
#include "qapi_usb.h"
#include "../demo/debug_port_cfg.h"

#ifdef ELOG_ASYNC_OUTPUT_ENABLE

TX_MUTEX 	 *elog_lockHandle;
TX_SEMAPHORE *elog_asyncHandle;
TX_SEMAPHORE *elog_dma_lockHandle;

#endif

extern void UART_Write(uint32_t Length, const char *Buffer);

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
	int Result;
	UINT status;
    ElogErrCode result = ELOG_NO_ERR;

	status = txm_module_object_allocate(&elog_lockHandle, sizeof(TX_MUTEX));
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to allocate elog_lockHandle: %d\r\n", status);
    }
	status = tx_mutex_create(elog_lockHandle,"elog_lockHandle",TX_NO_INHERIT);
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to create elog_lockHandle: %d\r\n", status);
    }

	status = txm_module_object_allocate(&elog_asyncHandle, sizeof(TX_SEMAPHORE));
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to allocate elog_asyncHandle: %d\r\n", status);
    }
	status = tx_semaphore_create(elog_asyncHandle,"elog_asyncHandle", 0);
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to create elog_asyncHandle: %d\r\n", status);
    }

	status = txm_module_object_allocate(&elog_dma_lockHandle, sizeof(TX_SEMAPHORE));
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to allocate elog_dma_lockHandle: %d\r\n", status);
    }
	status = tx_semaphore_create(elog_dma_lockHandle,"elog_dma_lockHandle", 0);
	if(status != TX_SUCCESS)
    {
      Debug_Printf("Failed to create elog_dma_lockHandle: %d\r\n", status);
    }
	
    return result;
}

/**
 * EasyLogger port deinitialize
 *
 * @return result
 */
ElogErrCode elog_port_deinit(void) {
    return ELOG_NO_ERR;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
	UART_Write(size, log);
#ifdef DEBUG_USE_UART
	/* UART uses async DMA — wait for TX completion callback */
	tx_semaphore_get(elog_dma_lockHandle, TX_WAIT_FOREVER);
#endif
	/* USB: qapi_USB_Write already returned after blocking write */
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    tx_mutex_get(elog_lockHandle, TX_WAIT_FOREVER);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    tx_mutex_put(elog_lockHandle);
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    
    /* add your code here */
     static char cur_system_time[16] = { 0 };
	 snprintf(cur_system_time, 16, "tick:%010d", (ULONG)tx_time_get());
	 return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    
    /* add your code here */
    return "";
}

#ifdef ELOG_ASYNC_OUTPUT_ENABLE

void elog_async_output_notice(void) 
{    
	tx_semaphore_put(elog_asyncHandle);
}

void Easylog_async_output_task(void *arg)
{
	size_t get_log_size = 0;
#ifdef ELOG_ASYNC_LINE_OUTPUT
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];
#else
    static char poll_get_buf[ELOG_ASYNC_OUTPUT_BUF_SIZE - 4];
#endif

    for(;;)
    {
        /* waiting log */
        tx_semaphore_get(elog_asyncHandle, TX_WAIT_FOREVER);
        /* polling gets and outputs the log */
        while (1) {
#ifdef ELOG_ASYNC_LINE_OUTPUT
            get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif
            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
}

#endif
