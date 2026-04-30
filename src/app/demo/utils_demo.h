/**
 * @file utils_demo.h
 * @brief Public utility declarations for demo modules.
 *
 * Declares allocation helpers, formatted logging helpers and the
 * bounded copy utility used across demo code.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __UTILS_DEMO_H__
#define __UTILS_DEMO_H__

#include "txm_module.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "qapi_diag.h"
#include "qflog_utils.h"
#include "qapi_types.h"

#define htons(s) ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))

/* Maximum length of log message */
#define MAX_DIAG_LOG_MSG_SIZE       512

/* Log message to Diag */
#define DAM_LOG_MSG_DIAG( lvl, ... )  \
  { \
    char log_diag_buf[ MAX_DIAG_LOG_MSG_SIZE ]; \
     \
    /* Format message for logging */  \
    dam_format_log_msg( log_diag_buf, MAX_DIAG_LOG_MSG_SIZE, __VA_ARGS__ );\
    \
    /* Log message to Diag */  \
    QAPI_MSG_SPRINTF( MSG_SSID_LINUX_DATA, lvl, "%s", log_diag_buf );  \
    \
    /* Log message to Terminal */  \
    QFLOG_MSG(MSG_SSID_DFLT, lvl, "%s", log_diag_buf); \
  }

#define LOG_INFO(...) DAM_LOG_MSG_DIAG(MSG_LEGACY_HIGH, __VA_ARGS__)

#define LOG_DEBUG(...) DAM_LOG_MSG_DIAG(MSG_LEGACY_MED, __VA_ARGS__)

#define LOG_ERROR(...) DAM_LOG_MSG_DIAG(MSG_LEGACY_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) DAM_LOG_MSG_DIAG(MSG_LEGACY_FATAL, __VA_ARGS__)

/**
 * @brief Allocate memory from the demo byte pool.
 * @param[in] size Number of bytes to allocate.
 * @return void* Pointer to allocated memory or NULL on failure.
 */
void *data_malloc(uint32_t size);

/**
 * @brief Free memory previously allocated with `data_malloc`.
 * @param[in] data Pointer returned by `data_malloc` (NULL-safe).
 */
void data_free(void *data);

/**
 * @brief Format a log message into a buffer (printf-style).
 * @param[out] buf_ptr Destination buffer to receive formatted string.
 * @param[in] buf_size Size of destination buffer in bytes.
 * @param[in] fmt printf-style format string.
 * @param[in] ... Format arguments.
 */
void dam_format_log_msg
(
 char *buf_ptr,
 int buf_size,
 char *fmt,
 ...
);

/**
 * @brief Bounded memory copy that copies at most the smaller of src/dst sizes.
 * @param[out] dst Destination buffer.
 * @param[in] dst_size Size of destination buffer.
 * @param[in] src Source buffer.
 * @param[in] src_size Size of source buffer.
 * @return UINT Number of bytes actually copied.
 */
UINT memscpy
(
  void   *dst,
  UINT   dst_size,
  const  void  *src,
  UINT   src_size
);


#endif