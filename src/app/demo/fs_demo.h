/**
 * @file fs_demo.h
 * @brief File system demo helper API.
 *
 * Public wrappers around QAPI file-system calls used by the demo
 * applications: open, read, write, seek, stat, rename, mkdir, rmdir,
 * and close.  Each helper logs the outcome via EasyLogger.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __FS_DEMO_H__
#define __FS_DEMO_H__

#include "../application/ThreadX.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert a QAPI FS error code to a human-readable string.
 *
 * @param[in] status  QAPI file-system status code.
 * @return Pointer to a static string describing the error.
 */
char *fs_demo_error_to_string(qapi_FS_Status_t status);

/**
 * @brief Open a file with an explicit mode.
 *
 * @param[in]  Path   Null-terminated file path.
 * @param[in]  Oflag  Open flags (QAPI_FS_O_*).
 * @param[in]  Mode   File permission mode bits.
 * @param[out] Fd_ptr Pointer to int that receives the file descriptor.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Open_With_Mode().
 */
qapi_Status_t fs_demo_Open_With_Mode(char *Path, int Oflag,
                                     qapi_FS_Mode_t Mode, int *Fd_ptr);

/**
 * @brief Open a file.
 *
 * @param[in]  Path   Null-terminated file path.
 * @param[in]  Oflag  Open flags (QAPI_FS_O_*).
 * @param[out] Fd_ptr Pointer to int that receives the file descriptor.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Open().
 */
qapi_Status_t fs_demo_Open(char *Path, int Oflag, int *Fd_ptr);

/**
 * @brief Read from an open file descriptor.
 *
 * @param[in]  Fd_ptr  File descriptor.
 * @param[out] Buf     Destination buffer.
 * @param[in]  Count   Maximum bytes to read.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Read().
 */
qapi_Status_t fs_demo_Read(int Fd_ptr, uint8 *Buf, uint32 Count);

/**
 * @brief Write to an open file descriptor.
 *
 * @param[in] Fd_ptr  File descriptor.
 * @param[in] Buf     Source buffer.
 * @param[in] Count   Number of bytes to write.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Write().
 */
qapi_Status_t fs_demo_Write(int Fd_ptr, uint8 *Buf, uint32 Count);

/**
 * @brief Close an open file descriptor.
 *
 * @param[in] Fd_ptr  File descriptor to close.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Close().
 */
qapi_Status_t fs_demo_Close(int Fd_ptr);

/**
 * @brief Rename a file or directory.
 *
 * @param[in] Old_Path  Current path.
 * @param[in] New_Path  New path.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Rename().
 */
qapi_Status_t fs_demo_Rename(char *Old_Path, char *New_Path);

/**
 * @brief Remove a directory.
 *
 * @param[in] Path  Directory path to remove.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Rm_Dir().
 */
qapi_Status_t fs_demo_Del_Dir(char *Path);

/**
 * @brief Create a directory.
 *
 * @param[in] Path  Directory path to create.
 * @param[in] Mode  Permission mode bits.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Mk_Dir().
 */
qapi_Status_t fs_demo_Mk_Dir(char *Path, qapi_FS_Mode_t Mode);

/**
 * @brief Get file/directory statistics.
 *
 * @param[in] Path  Null-terminated path to stat.
 * @return File size in bytes on success; 0 on failure.
 */
uint32 fs_demo_Stat(const char *Path);

/**
 * @brief Seek to a byte offset within a file.
 *
 * @param[in] Fd_ptr  File descriptor.
 * @param[in] offset  Byte offset from the start of the file.
 * @retval QAPI_OK  Success.
 * @retval Other    Error code from qapi_FS_Seek().
 */
qapi_Status_t fs_demo_seek(int Fd_ptr, long long offset);

#ifdef __cplusplus
}
#endif

#endif /* __FS_DEMO_H__ */
