/**
 * @file fs_demo.h
 * @brief Public prototypes for the file system demo helpers.
 *
 * Declarations for the FS helper functions implemented in fs_demo.c.
 * These are thin wrappers around the QAPI FS APIs used by the demos.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __FS_DEMO_H__
#define __FS_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
qapi_Status_t fs_demo_Open_With_Mode(char *Path, int Oflag,qapi_FS_Mode_t Mode, int Fd_ptr);
qapi_Status_t fs_demo_Open(char *Path, int Oflag, int Fd_ptr);
qapi_Status_t fs_demo_Read(int Fd_ptr, uint8 * Buf, uint32 Count);
qapi_Status_t fs_demo_Write(int Fd_ptr, uint8 * Buf, uint32 Count);
qapi_Status_t fs_demo_Close(int Fd_ptr);
qapi_Status_t fs_demo_Rename(char * Old_Path, char * New_Path);
qapi_Status_t fs_demo_Del_Dir(char * Path);
qapi_Status_t fs_demo_Mk_Dir(char * Path, qapi_FS_Mode_t Mode);
uint32 fs_demo_Stat(const char* Path);
qapi_Status_t fs_demo_seek(int Fd_ptr, long long offset);
#endif

