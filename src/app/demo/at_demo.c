/**
 * @file at_demo.c
 * @brief AT command channel demo utilities.
 *
 * Provides helpers to open the visual AT channel, send AT commands and
 * process asynchronous AT output for demo purposes. The module registers
 * a lightweight output callback used by the DAM core and posts semaphores
 * when specific responses arrive (eg. update notifications).
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)   
	#define LOG_TAG    "AT_demo"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>


#include "at_demo.h"
#include "qapi_dam.h"

#include "../Easylogger/elog.h"


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/
extern TX_SEMAPHORE *Update_semaphore;

unsigned char data[2048] = {0};  
unsigned char tempBuf[256] = {0};  


/**
  * @brief  callback for AT command response output.
  * @param  
  * @note   the stack in this callback func is limited, please don't use big array inside.
  * @retval 
  */
static void At_Output_Cb(void)
{
   int len = 0;  
   int outLen = 0;
   int i = 0;
	
	memset(data,0,2048);   
	do
	{
		len = qapi_DAM_Visual_AT_Output((unsigned char *)&data[outLen],2048-outLen);
		outLen += len;
		log_i("At_Output_Cb,outLen=%d", outLen); 
		if (outLen > 2048)
		{
			break;
		}
	}while(len > 0);
		
	if (strstr(data,"+HTTPTOFS: 200") != NULL)
	{
		tx_semaphore_put(Update_semaphore);			//if get +HTTPTOFS: 200, meaning there is new app file downloaded, will start update app thread.
	}
		
	if (outLen > 0)
	{
		do 
		{
			memset(tempBuf,0,256);
			if (outLen > 255)
			{
				memcpy(tempBuf,&data[i],255);
				i += 255;
				outLen -= 255;
			}
			else
			{
				memcpy(tempBuf,&data[i],strlen((char *)&data[i]));
				outLen = 0;
			}
			log_i("%s",tempBuf);		//print the data

		}while(outLen > 0);
	}
}

/**
  * @brief  send AT command to the DAM core.
  * @param  ATCMD -- AT command string(no need \r\n, the API will add it in the end automatically)
  * @note   After open the AT channel, need to flush all previously recerived data by qapi_DAM_Visual_AT_Output, or AT command channel will get blocked so callback will work abnormally.
  * @retval DAM_Status_t -- DAM_STATUS_SUCCESS or DAM_STATUS_ERROR.
  */
DAM_Status_t atcmd(uint8_t *ATCMD)
{   
	unsigned char data[1024] = {0};       
	static unsigned char s_at_open=0,s_start=0;
	int len=0;
	if(!s_at_open)
	{
		s_at_open=1;
		qapi_DAM_Visual_AT_Open(At_Output_Cb);
		log_i("AT Opened");
	}
	else
	{
		log_i("AT already Opened");
	}

	if(!s_start)
	{
		s_start=1;
		while(len = qapi_DAM_Visual_AT_Output((unsigned char *)&data[0],1024))
			log_i("DAM AT Read len=%d %s",len,data);
	}

	memset(data,0,1024);   
	memcpy(data,ATCMD,strlen((char *)ATCMD));
	log_i("atcmd is:%s", data); 
	memcpy(data+strlen((char *)data),"\r\n",2);
	qapi_DAM_Visual_AT_Input(data,strlen((char *)data));

	return DAM_STATUS_SUCCESS;   
}


