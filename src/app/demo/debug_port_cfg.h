/**
 * @file debug_port_cfg.h
 * @brief Debug output port selection (shared by debug_uart.c and elog_port.c).
 *
 * Define exactly one of:
 *   DEBUG_USE_USB  - USB virtual serial (blocking write, no DMA callback)
 *   DEBUG_USE_UART - hardware UART (async DMA with TX completion callback)
 *
 * When DEBUG_USE_USB is active:
 *   - UART_Write() routes to qapi_USB_Write() (blocking, returns after write)
 *   - EasyLogger async task runs — reads ring buffer, writes via USB (blocking)
 *   - elog_port_output() calls qapi_USB_Write() directly, no DMA semaphore wait
 *   - log_i()/log_e() are fast (write to ring buffer), async task handles USB I/O
 *
 * When DEBUG_USE_UART is active:
 *   - UART_Write() uses qapi_UART_Transmit() (async DMA)
 *   - EasyLogger async task runs — reads ring buffer, writes via UART DMA
 *   - elog_port_output() waits on elog_dma_lockHandle after UART_Write()
 */

#ifndef DEBUG_PORT_CFG_H
#define DEBUG_PORT_CFG_H

#define DEBUG_USE_USB
/* #define DEBUG_USE_UART */

#if defined(DEBUG_USE_USB) && defined(DEBUG_USE_UART)
#error "Define only one of DEBUG_USE_USB or DEBUG_USE_UART"
#endif
#if !defined(DEBUG_USE_USB) && !defined(DEBUG_USE_UART)
#error "Define one of DEBUG_USE_USB or DEBUG_USE_UART"
#endif

#endif /* DEBUG_PORT_CFG_H */
