/**
 * @file demo_utils.c
 * @brief General demo utility helpers (alloc/free/log formatting, memscpy).
 *
 * Small utility wrappers used by demo modules: allocation from the demo
 * byte pool, safe free, formatted log message helper and a bounded
 * memory copy function.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#include "utils_demo.h"
#include "../Easylogger/elog.h"

extern TX_BYTE_POOL *byte_pool_dam;

/**
 * @brief Allocate memory from the demo byte pool.
 *
 * Allocates `size` bytes from `byte_pool_dam` and zero-initializes the
 * returned buffer. Returns NULL on allocation failure.
 *
 * @param[in] size Number of bytes to allocate.
 * @return void* Pointer to allocated memory or NULL on failure.
 */
void *data_malloc(uint32_t size)
{
  void *data = NULL;
  uint32_t status = 0;

  if (0 == size)
  {
    return NULL;
  }

  status = tx_byte_allocate(byte_pool_dam, (VOID **)&data, size, TX_NO_WAIT);

  if (TX_SUCCESS != status)
  {
    log_i("DAM_APP:Failed to allocate memory with %d", status); 
    return NULL;
  }

  if(NULL != data)
  {
    memset(data, 0, size);
  }

  return data;
}

/**
 * @brief Release memory previously allocated with data_malloc().
 *
 * @param[in] data  Pointer returned by data_malloc().  Passing NULL
 *                  is safe and results in a no-op.
 */
void data_free(void *data)
{
    uint32_t status;

    if (NULL == data)
    {
        return;
    }

    status = tx_byte_release(data);

    if (TX_SUCCESS != status)
    {
        log_e("DAM_APP: Failed to release memory, status=%d", status);
    }
    /* Note: setting a local copy to NULL has no effect on the caller.
     * Callers should NULL their own pointer after calling data_free(). */
}

/**
 * @brief Format a log message into a buffer.
 *
 * Variadic wrapper around `vsnprintf` that validates the destination buffer
 * before formatting.
 *
 * @param[out] buf_ptr Destination buffer to receive formatted string.
 * @param[in] buf_size Size of destination buffer in bytes.
 * @param[in] fmt printf-style format string.
 * @param[in] ... Format arguments.
 */
void dam_format_log_msg(char *buf_ptr, int buf_size, char *fmt, ...)
{
  va_list ap;

  /* validate input param */
  if( NULL == buf_ptr || buf_size <= 0)
  {
    log_i("DAM_APP:dam_format_log_msg: Bad Param buf_ptr:[%p], buf_size:%d",buf_ptr, buf_size);
    return;
  }

  va_start( ap, fmt );

  vsnprintf( buf_ptr, (size_t)buf_size, fmt, ap );

  va_end( ap );

}

/**
 * @brief Bounded memory copy.
 *
 * Copies up to `min(dst_size, src_size)` bytes from `src` to `dst` and
 * returns the number of bytes copied.
 *
 * @param[out] dst Destination buffer.
 * @param[in] dst_size Size of destination buffer.
 * @param[in] src Source buffer.
 * @param[in] src_size Size of source buffer.
 * @return UINT Number of bytes actually copied.
 */
UINT memscpy(void *dst, UINT dst_size, const void  *src, UINT  src_size)
{
  UINT  copy_size = (dst_size <= src_size)? dst_size : src_size;
  memcpy(dst, src, copy_size);
  return copy_size;
}
