/**
 * @file update_demo.c
 * @brief App update demo helpers.
 *
 * Utilities for validating update images, computing CRC32 and checking
 * update state on the demo filesystem.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#if !defined(LOG_TAG)
	#define LOG_TAG    "Update_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "qapi_fs_types.h"
#include "qapi_fs.h"

#include "update_demo.h"

 extern void Debug_Printf(char* format, ...);

/**
 * @brief Compute CRC32 (IEEE 802.3 polynomial) over a buffer.
 *
 * @param[in] data Pointer to input buffer.
 * @param[in] length Length of the buffer in bytes.
 * @return uint32 Calculated CRC32 checksum.
 */
uint32 crc32(uint8_t *data, uint32 length)
{

	uint8 i;

	uint32 crc = 0xffffffff;		  // Initial value

	while(length--)

	{

		crc ^= *data++; 			   // crc ^= *data; data++;

		for (i = 0; i < 8; ++i)

		{

			if (crc & 1)

				crc = (crc >> 1) ^ 0xEDB88320;// 0xEDB88320= reverse 0x04C11DB7

			else

				crc = (crc >> 1);

		}

	}
	
	Debug_Printf("CRC:0x%x;~CRC:0x%x\r\n",crc,~crc);
	return ~crc;

}

/**
 * @brief Verify a file using its trailing CRC32 and truncate on success.
 *
 * Reads the file at `Path` into `Buf` (up to `Buf_size`), computes the CRC32
 * over the file contents excluding the last 4 bytes, and compares against the
 * trailing CRC stored in the file. If the CRC matches the file is truncated
 * to remove the trailing CRC and success is returned.
 *
 * @param[in] Path Path to the file to verify.
 * @param[out] Buf Buffer provided by caller to read file contents into.
 * @param[in] Buf_size Size of the provided buffer in bytes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS if CRC matches; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t file_check(char *Path,uint8 * Buf,uint32 Buf_size)
{
	int Fd_ptr;
	uint32 len=0;
	uint32 CRC=0;
	char crc_buf[4] = {0};
	uint32 i=0,outLen=0;

	memset(crc_buf, 0, 4);
	memset(Buf, 0, Buf_size);
	
	
	len = fs_demo_Stat(Path);
	outLen = len;
	
	fs_demo_Open(Path, QAPI_FS_O_RDWR_E, Fd_ptr);
	do 
	{
		if (outLen > 32767)
		{
			fs_demo_Read(Fd_ptr, Buf+i, 32767);
			i += 32767;
			outLen -= 32767;
		}
		else
		{	
			fs_demo_Read(Fd_ptr, Buf+i, outLen);
			outLen = 0;
		}
		
	}while(outLen > 0);
	fs_demo_Close(Fd_ptr);

	memcpy(crc_buf,Buf+(len-4),4);
	CRC = ((uint32)crc_buf[0] << 24) | ((uint32)crc_buf[1] << 16) | ((uint32)crc_buf[2] << 8) | ((uint32)crc_buf[3]);
	Debug_Printf("CRC_BUF=:0x%x,0x%x,0x%x,0x%x\r\n",crc_buf[0],crc_buf[1],crc_buf[2],crc_buf[3]);
	Debug_Printf("CRC:0x%x\r\n",CRC);
	if (CRC == crc32(Buf, len-4))
		{
		qapi_FS_Truncate(Path, len-4);
		Debug_Printf("file check successful!!\r\n");
		return DAM_STATUS_SUCCESS;
		}
	else
		{
		//qapi_FS_Truncate(Path, 0);
		Debug_Printf("file check failed!!\r\n");
		return DAM_STATUS_ERROR;
		}
	

}

/**
 * @brief Check the application update result state.
 *
 * Determines whether the last update completed successfully by checking
 * presence of update artifacts in the filesystem.
 *
 * @return DAM_Status_t DAM_STATUS_SUCCESS if update succeeded; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t update_state_check(VOID)
{
	int Fd_ptr;
	
	if (qapi_FS_Open("/custapp/update.log", QAPI_FS_O_RDWR_E, &Fd_ptr)==QAPI_OK)
		{
		fs_demo_Close(Fd_ptr);
		if (qapi_FS_Open("/custapp/cust_app.update", QAPI_FS_O_RDWR_E, &Fd_ptr) == QAPI_ERR_NO_ENTRY)
			{
			Debug_Printf("update successful!!\r\n");
		    return DAM_STATUS_SUCCESS;
			}
		else
			{
			fs_demo_Close(Fd_ptr);
			Debug_Printf("update failed!!\r\n");
		    return DAM_STATUS_ERROR;
			}
				
		}
	else
		{
		Debug_Printf("update failed!!\r\n");
		return DAM_STATUS_ERROR;
		}

}


