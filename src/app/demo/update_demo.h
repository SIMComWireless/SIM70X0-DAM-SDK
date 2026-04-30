/**
 * @file update_demo.h
 * @brief Public declarations for the app update helpers.
 *
 * Declares CRC and file verification helpers used by the update demo
 * implementation.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __UPDATE_DEMO_H__
#define __UPDATE_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Compute CRC32 checksum over a buffer.
 * @param[in] data Input data buffer.
 * @param[in] length Length of the buffer in bytes.
 * @return uint32 CRC32 checksum.
 */
uint32 crc32(uint8_t *data, uint32 length);

/**
 * @brief Verify a file's trailing CRC and optionally truncate it.
 * @param[in] Path Path to the file to verify.
 * @param[out] Buf Buffer to use for file reads (caller-provided).
 * @param[in] Buf_size Size of the provided buffer in bytes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on valid file; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t file_check(char *Path,uint8 * Buf,uint32 Buf_size);

/**
 * @brief Check the system's update state (post-update verification).
 * @return DAM_Status_t DAM_STATUS_SUCCESS if update succeeded; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t update_state_check(VOID);

#endif

