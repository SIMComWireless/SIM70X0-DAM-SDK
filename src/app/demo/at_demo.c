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
 *
 * NOTE: `data` is intentionally global because At_Output_Cb() runs in
 *       a callback context with a very limited stack.  Access must be
 *       serialised by the caller (only one AT transaction at a time).
 *-----------------------------------------------------------------------*/
extern TX_SEMAPHORE *Update_semaphore;

/** @brief Buffer for AT response collection (limited-stack callback). */
static unsigned char data[2048] = {0};

/** @brief Chunk buffer for logging AT responses line by line. */
static unsigned char tempBuf[256] = {0};  


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
 * @brief Send an AT command via the DAM visual AT channel.
 *
 * Opens the visual AT channel on first call, flushes any stale data,
 * then sends the command appended with CRLF.
 *
 * @param[in] ATCMD  Null-terminated AT command string (CRLF is appended
 *                   automatically).
 *
 * @retval DAM_STATUS_SUCCESS  Command sent.
 *
 * @note After opening the AT channel the function flushes all previously
 *       received data via qapi_DAM_Visual_AT_Output(); otherwise the
 *       channel gets blocked and the callback works abnormally.
 */
DAM_Status_t atcmd(uint8_t *ATCMD)
{
    /* Must be static — the DAM framework may hold a pointer to this
     * buffer across calls, and a stack-local would be invalidated. */
    static unsigned char at_cmd[1024] = {0};
    static unsigned char s_at_open = 0;
    static unsigned char s_start  = 0;
    int len = 0;

    if (!s_at_open)
    {
        s_at_open = 1;
        qapi_DAM_Visual_AT_Open(At_Output_Cb);
        log_i("AT channel opened");
    }

    /* First-time flush: drain any stale data left in the AT pipe so
     * the callback does not get blocked by old responses. */
    if (!s_start)
    {
        s_start = 1;
        memset(at_cmd, 0, sizeof(at_cmd));
        while ((len = qapi_DAM_Visual_AT_Output(at_cmd, sizeof(at_cmd) - 1)) > 0)
        {
            at_cmd[len] = '\0';   /* null-terminate for safe logging */
            log_i("AT flush [%d]: %s", len, at_cmd);
            memset(at_cmd, 0, sizeof(at_cmd));
        }
    }

    /* Build and send the command */
    memset(at_cmd, 0, sizeof(at_cmd));
    len = strlen((char *)ATCMD);
    if (len + 2 > (int)sizeof(at_cmd))
    {
        log_e("AT command too long (%d bytes)", len);
        return DAM_STATUS_ERROR;
    }
    memcpy(at_cmd, ATCMD, len);
    at_cmd[len]     = '\r';
    at_cmd[len + 1] = '\n';
    log_i("AT send: %s", ATCMD);
    qapi_DAM_Visual_AT_Input(at_cmd, len + 2);

    return DAM_STATUS_SUCCESS;
}


