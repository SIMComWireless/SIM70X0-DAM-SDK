/**
 * @file fs_demo.c
 * @brief File system demo helpers (open/read/write/seek/etc.).
 *
 * Wrappers around the QAPI FS APIs to simplify file operations in demos.
 * Each helper logs the result and returns the QAPI status for callers to
 * handle. Error mapping helpers provide readable messages for diagnostics.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "FS_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_fs_types.h"
#include "qapi/qapi_fs.h"

#include "fs_demo.h"
#include "../Easylogger/elog.h"


/**
  * @brief  FS demo error code print string.
  * @param  status --string match according to status code.
  * @note   
  * @retval err_string --string for status code.
  */
char * fs_demo_error_to_string(qapi_FS_Status_t status)
{
    char * err_string;
    switch(status)
    {
        case QAPI_ERR_EXISTS:
            err_string = "Another file with the same name exists";
            break;
        case QAPI_ERR_NO_ENTRY:
            err_string = "No entry for the path name is found";
            break;
        case QAPI_ERR_EMFILE:
            err_string = "Maximum number of open descriptors is exceeded";
            break;
        case QAPI_ERR_EISDIR:
            err_string = "Opening a file with a write flag (QAPI_FS_O_WRONLY or QAPI_FS_O_RDWR) was denied because a directory with the same name exists.";
            break;
        case QAPI_ERR_ENOSPC:
            err_string = "No space is left on the device.";
            break;
        case QAPI_ERR_ENAMETOOLONG:
            err_string = "Name is too long.";
           break;
        case QAPI_ERR_NO_MEMORY:
            err_string = "No more dynamic memory is available.";
            break;
        case QAPI_ERR_ENODEV:
            err_string = "The device does not exist.";
            break;
        case QAPI_ERR_ENOTDIR:
            err_string = "The file could not be created under a path that is not a directory.";
            break;
        case QAPI_ERR_INVALID_PARAM:
            err_string = "Invalid parameter or path.";
            break;
        case QAPI_ERR_UNKNOWN:
            err_string = "Unknown";
            break;
        default:
            err_string = "Unknown";
            break;
    }
    return err_string;
}

/**
 * @brief Open a file with an explicit mode.
 *
 * Wrapper around qapi_FS_Open_With_Mode() that logs success or failure.
 *
 * @param[in]  Path   Null-terminated path to the file to open.
 * @param[in]  Oflag  Open flags (QAPI_FS_O_*).
 * @param[in]  Mode   File mode bits (qapi_FS_Mode_t).
 * @param[out] Fd_ptr Pointer to int that receives the file descriptor.
 *
 * @retval QAPI_OK  File opened successfully.
 * @retval Other    qapi_FS_Open_With_Mode() error code.
 */
qapi_Status_t fs_demo_Open_With_Mode(char *Path, int Oflag, qapi_FS_Mode_t Mode, int *Fd_ptr)
{
    qapi_Status_t status;

    status = qapi_FS_Open_With_Mode(Path, Oflag, Mode, Fd_ptr);
    if (status != QAPI_OK)
    {
        log_e("open with mode failed: %s", fs_demo_error_to_string(status));
    }
    else
    {
        log_i("open with mode succeeded, fd=%d", *Fd_ptr);
    }
    return status;
}

/**
 * @brief Open a file.
 *
 * Convenience wrapper around qapi_FS_Open() that logs success or failure.
 *
 * @param[in]  Path   Null-terminated path to the file to open.
 * @param[in]  Oflag  Open flags (QAPI_FS_O_*).
 * @param[out] Fd_ptr Pointer to int that receives the file descriptor.
 *
 * @retval QAPI_OK  File opened successfully.
 * @retval Other    qapi_FS_Open() error code.
 */
qapi_Status_t fs_demo_Open(char *Path, int Oflag, int *Fd_ptr)
{
    qapi_Status_t status;

    status = qapi_FS_Open(Path, Oflag, Fd_ptr);
    if (status != QAPI_OK)
    {
        log_e("open file failed: %s", fs_demo_error_to_string(status));
    }
    else
    {
        log_i("open file succeeded, fd=%d", *Fd_ptr);
    }
    return status;
}

/**
 * @brief Read from a file descriptor.
 *
 * Reads up to @p Count bytes into @p Buf using qapi_FS_Read() and logs
 * the actual number of bytes read.
 *
 * @param[in]  Fd_ptr  File descriptor returned from fs_demo_Open().
 * @param[out] Buf     Buffer to receive data (must be at least @p Count bytes).
 * @param[in]  Count   Maximum number of bytes to read.
 *
 * @retval QAPI_OK  Read successful.
 * @retval Other    qapi_FS_Read() error code.
 */
qapi_Status_t fs_demo_Read(int Fd_ptr, uint8 * Buf, uint32 Count)
{ 
    uint32 Bytes_Read_Ptr;
    qapi_Status_t status;
    
    status = qapi_FS_Read(Fd_ptr,Buf,Count,&Bytes_Read_Ptr);
    if (status != QAPI_OK)
    {
        log_i("read file failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("read file succeeded !!!");
		log_i("the actual length of read is %d,%.*s",Bytes_Read_Ptr,Bytes_Read_Ptr,Buf);
    }
    return status;
}

/**
 * @brief Write to a file descriptor.
 *
 * Writes @p Count bytes from @p Buf to @p Fd_ptr using qapi_FS_Write()
 * and logs the actual number of bytes written.
 *
 * @param[in] Fd_ptr  File descriptor returned from fs_demo_Open().
 * @param[in] Buf     Buffer containing data to write.
 * @param[in] Count   Number of bytes to write.
 *
 * @retval QAPI_OK  Write successful.
 * @retval Other    qapi_FS_Write() error code.
 */
qapi_Status_t fs_demo_Write(int Fd_ptr, uint8 * Buf, uint32 Count)
{
    
    uint32 Bytes_Written_Ptr = 0;
    qapi_Status_t status;
	
    status = qapi_FS_Write(Fd_ptr,Buf,Count,&Bytes_Written_Ptr);
    if (status != QAPI_OK)
    {
        log_i("write file failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("write file succeeded !!!");
		log_i("the actual length of writing is %d",Bytes_Written_Ptr);
    }
    return status;
}

/**
 * @brief Close an open file descriptor.
 *
 * @param[in] Fd_ptr  File descriptor to close.
 *
 * @retval QAPI_OK  File closed successfully.
 * @retval Other    qapi_FS_Close() error code.
 */
qapi_Status_t fs_demo_Close(int Fd_ptr)
{
    qapi_Status_t status;
	
    status = qapi_FS_Close(Fd_ptr);//0,1,2.... value is Open return fd
    if (status != QAPI_OK)
    {
        log_i("close file failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("close file succeeded !!!");
    }
    return status;
}

/**
 * @brief Rename a file or directory.
 *
 * @param[in] Old_Path  Current path of the file/directory.
 * @param[in] New_Path  New desired path.
 *
 * @retval QAPI_OK  Renamed successfully.
 * @retval Other    qapi_FS_Rename() error code.
 */
qapi_Status_t fs_demo_Rename(char * Old_Path, char * New_Path)
{
    qapi_Status_t status;
	
    status = qapi_FS_Rename(Old_Path,New_Path);
    if (status != QAPI_OK)
    {
        log_i("rename file failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("rename file succeeded !!!");
    }
    return status;
}

/**
 * @brief Remove a directory.
 *
 * @param[in] Path  Directory path to remove.
 *
 * @retval QAPI_OK  Directory removed successfully.
 * @retval Other    qapi_FS_Rm_Dir() error code.
 */
qapi_Status_t fs_demo_Del_Dir(char * Path)
{
    qapi_Status_t status;
	
    status = qapi_FS_Rm_Dir(Path);
    if (status != QAPI_OK)
    {
        log_i("del dir failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("del dir succeeded !!!");
    }
    return status;
}

/**
 * @brief Create a directory with the specified mode.
 *
 * @param[in] Path  Directory path to create.
 * @param[in] Mode  Mode flags for the created directory.
 *
 * @retval QAPI_OK  Directory created successfully.
 * @retval Other    qapi_FS_Mk_Dir() error code.
 */
qapi_Status_t fs_demo_Mk_Dir(char * Path, qapi_FS_Mode_t Mode)
{
    qapi_Status_t status;
	
    status = qapi_FS_Mk_Dir(Path,Mode);
    if (status != QAPI_OK)
    {
        log_i("mk dir failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("mk dir succeeded !!!");
    }
    return status;
}

/**
 * @brief Get file/directory statistics (size, inode, etc.).
 *
 * Wraps qapi_FS_Stat() and returns the file size on success.
 *
 * @param[in] Path  Null-terminated path to file/directory to stat.
 *
 * @return File size in bytes on success; 0 on failure (check logs).
 */
uint32 fs_demo_Stat(const char* Path)
{
    qapi_Status_t status;
	struct qapi_FS_Stat_Type_s SBuf;
	
    status = qapi_FS_Stat(Path,&SBuf);
    if (status != QAPI_OK)
    {
        log_i("get stat failed !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("get stat succeeded: st_ino=%d, st_size=%d", SBuf.st_ino, SBuf.st_size);
        return SBuf.st_size;
    }
    return 0;
}


/**
 * @brief Seek to an offset within a file.
 *
 * Uses qapi_FS_Seek() to set the file position relative to the start.
 *
 * @param[in] Fd_ptr  File descriptor to seek.
 * @param[in] offset  Byte offset from the beginning of the file.
 *
 * @retval QAPI_OK  Seek successful.
 * @retval Other    qapi_FS_Seek() error code.
 */
qapi_Status_t fs_demo_seek(int Fd_ptr, long long offset)
{
    qapi_Status_t status;

    qapi_FS_Offset_t actual_offset;
	
    status = qapi_FS_Seek(Fd_ptr , offset , QAPI_FS_SEEK_SET_E, &actual_offset);
    if (status != QAPI_OK)
    {
        log_i("file seek !!! %s",fs_demo_error_to_string(status));
    }
    else
    {
        log_i("actual_offset:%d",actual_offset);
        log_i("file seek succeeded !!!");
    }
    return status;
}
