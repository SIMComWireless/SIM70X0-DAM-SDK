/**
 * @file socket_demo.c
 * @brief TCP/UDP socket demo helpers.
 *
 * Simple blocking TCP/UDP client helpers used by demo code to connect,
 * send and receive data using QAPI sockets. Includes a small helper to
 * convert dotted-quad string addresses to 32-bit form.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#if !defined(LOG_TAG)
	#define LOG_TAG    "Socket_demo"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "qapi.h"
#include "qapi_status.h"
#include "qapi_types.h"
#include "qapi_fs.h"
#include "qapi_timer.h"
#include "msgcfg.h"
#include "msg_mask.h"
#include "qapi_diag.h"
#include "qapi_dss.h"
#include "qapi_socket.h"
#include "qapi_dnsc.h"
#include "qapi_ns_utils.h"
#include "qapi_ssl.h"
#include "txm_module.h"

#include "../demo/netutils.h"
#include "socket_demo.h"
#include "../Easylogger/elog.h"

/**
 * @brief Convert dotted-decimal string to 32-bit IP representation.
 *
 * Parses an IPv4 address in dotted-quad ASCII form ("a.b.c.d") and
 * writes the 32-bit value to `ipaddr` in host order.
 *
 * @param[in] buf Null-terminated dotted-decimal IPv4 string.
 * @param[out] ipaddr Pointer to uint32 where converted address will be stored.
 *
 * @return uint32 True (non-zero) on success, false (0) on parse error.
 */
static uint32 simcom_ipaddr8_to_32(uint8 *buf,uint32 *ipaddr)
{
    uint8      c = 0;
    uint8      i = 0;
    uint8      j = 0;
    bool    result = true; 
    uint8      length = strlen((char *)buf);
    uint8      ip[4];
    uint8      cnt = 0;
    uint16     temp_ip = 0;     

    while(i < length)
    {
        c = buf[i++];
        if((c >= '0') && (c <= '9'))
        {
            temp_ip = temp_ip * 10 + (c - '0');
            if(temp_ip > 255)
            {
                result = false;
                break;
            }
        }
        else if((c == '.' ) && (temp_ip < 256)) 
        {
            ip[j++] = (uint8)temp_ip;
            cnt ++;
            temp_ip = 0;
        }
        else
        {
            result = false;
            break;
        }
    }

    if(result == true)
    {
        ip[j] = (uint8)temp_ip;
        cnt++;
        
        if(cnt == 4)
        {
            *ipaddr =  ((((uint32)(ip[3]<<24))&0xff000000) | (((uint32)(ip[2]<<16))&0x00ff0000) | (((uint32)(ip[1]<<8))&0x0000ff00) | (((uint32)ip[0])&0x000000ff));
        }
        else
        {
            result =false;
        }
    }

    return result;
}

/**
 * @brief Simple blocking TCP client demo.
 *
 * Creates a TCP socket, connects to @p url : @p port, sends @p data,
 * waits for a single response, then closes the socket.
 *
 * @param[in] url   IPv4 address string (dotted-quad).
 * @param[in] port  Destination TCP port.
 * @param[in] data  Null-terminated data buffer to send.
 *
 * @retval DAM_STATUS_SUCCESS  Transaction completed successfully.
 * @retval DAM_STATUS_ERROR    Connect, send, or receive failed.
 */
DAM_Status_t socket_demo_TCP(char *url, int port, char *data)
{
    int tcp_fd;
    struct sockaddr_in dest_addr;
    DAM_Status_t result = DAM_STATUS_ERROR;

    memset(&dest_addr, 0x00, sizeof(struct sockaddr_in));

    tcp_fd = qapi_socket(AF_INET, SOCK_STREAM, 0);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    simcom_ipaddr8_to_32((uint8 *)url, (uint32 *)&dest_addr.sin_addr.s_addr);
    memset(dest_addr.sin_zero, 0x00, 8);

    if (0 == qapi_connect(tcp_fd, (struct sockaddr *)&dest_addr,
                          sizeof(dest_addr)))
    {
        int len = strlen(data);
        int bytes_send = qapi_send(tcp_fd, data, len, 0);
        if (bytes_send > 0)
        {
            log_i("Socket_demo_TCP send %d bytes", bytes_send);
            char buf[512] = {0};
            int bytes_recv = qapi_recv(tcp_fd, buf, 512, 0);
            if (bytes_recv > 0)
            {
                log_i("TCP recv: len=%d, data=%s", bytes_recv, buf);
                result = DAM_STATUS_SUCCESS;
            }
            else
            {
                log_e("TCP recv error");
            }
        }
        else
        {
            log_e("TCP send error");
        }
    }
    else
    {
        log_e("TCP connect error");
    }

    qapi_socketclose(tcp_fd);
    log_i("TCP Socket Closed");
    return result;
}

/**
 * @brief Simple blocking UDP demo.
 *
 * Creates a UDP socket, sends @p data to @p url : @p port via sendto,
 * performs a single recvfrom for the response, then closes the socket.
 *
 * @param[in] url   IPv4 address string (dotted-quad).
 * @param[in] port  Destination UDP port.
 * @param[in] data  Null-terminated data buffer to send.
 *
 * @retval DAM_STATUS_SUCCESS  Transaction completed successfully.
 * @retval DAM_STATUS_ERROR    Send or receive failed.
 */
DAM_Status_t socket_demo_UDP(char *url, int port, char *data)
{
    int udp_fd;
    struct sockaddr_in dest_addr;
    DAM_Status_t result = DAM_STATUS_ERROR;

    memset(&dest_addr, 0x00, sizeof(struct sockaddr_in));
    udp_fd = qapi_socket(AF_INET, SOCK_DGRAM, 0);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    simcom_ipaddr8_to_32((uint8 *)url, (uint32 *)&dest_addr.sin_addr.s_addr);
    memset(dest_addr.sin_zero, 0x00, 8);

    {
        int len = strlen(data);
        int bytes_send = qapi_sendto(udp_fd, data, len, 0,
                                     (struct sockaddr *)&dest_addr,
                                     sizeof(dest_addr));
        if (bytes_send > 0)
        {
            log_i("Socket_demo_UDP send %d bytes", bytes_send);
            char buf[512] = {0};
            int fromlen = sizeof(struct sockaddr);
            int bytes_recv = qapi_recvfrom(udp_fd, buf, 512, 0,
                                           (struct sockaddr *)&dest_addr,
                                           &fromlen);
            if (bytes_recv > 0)
            {
                log_i("UDP recv: len=%d, data=%s", bytes_recv, buf);
                result = DAM_STATUS_SUCCESS;
            }
            else
            {
                log_e("UDP recv error");
            }
        }
        else
        {
            log_e("UDP send error=%d", qapi_errno(udp_fd));
        }
    }

    qapi_socketclose(udp_fd);
    log_i("UDP Socket Closed");
    return result;
}




