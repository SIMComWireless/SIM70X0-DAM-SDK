/**
 * @file mqtt_demo.c
 * @brief MQTT client demo helpers.
 *
 * Wraps the QAPI MQTT APIs to provide simple connect/subscribe/publish
 * helpers for demo applications, including callback handlers and
 * configuration utilities.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)    
	#define LOG_TAG    "Mqtt_demo"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "qapi/qapi_status.h"
#include "qapi/qapi_ns_utils.h"
#include "qapi/qapi_socket.h"

#include "qapi/qapi_mqtt.h"
#include "qapi_dnsc.h"
#include "qapi_dss.h"

#include "mqtt_demo.h"
#include "utils_demo.h"

#include "../Easylogger/elog.h"


extern TX_BYTE_POOL * byte_pool_dam;

bool is_mqtt_init=false;

qapi_Net_MQTT_Hndl_t mqtt_handle;

qapi_Net_MQTT_Config_t mqtt_config; 

#define IP_ADDR_SIZE 48

/**
 * @brief Configure MQTT connection parameters.
 *
 * Populates the global `mqtt_config` structure used by the MQTT client.
 *
 * @param[in] CLIENT_ID Null-terminated client identifier string.
 * @param[in] KEEPALIVE_TIME Keepalive interval in seconds.
 * @param[in] CLEAN_SESSION Non-zero to request clean session.
 * @param[in] USERNAME Optional username (may be NULL).
 * @param[in] PASSWORD Optional password (may be NULL).
 *
 * @return int 0 on success, non-zero on error.
 */
int mqtt_config_set(char* CLIENT_ID,uint32_t KEEPALIVE_TIME,uint8_t CLEAN_SESSION,uint8_t* USERNAME, uint8_t* PASSWORD) 
{
  memset(&mqtt_config, 0, sizeof(mqtt_config));
  strcpy((char *)mqtt_config.client_id,CLIENT_ID);
  mqtt_config.client_id_len = strlen(CLIENT_ID);
  mqtt_config.nonblocking_connect = false;
  mqtt_config.keepalive_duration =KEEPALIVE_TIME; // qurt_timer_convert_time_to_ticks(k_alive, QURT_TIME_SEC);
  mqtt_config.clean_session = CLEAN_SESSION;
  mqtt_config.will_topic =NULL;
  mqtt_config.will_topic_len =0;
  mqtt_config.will_message =NULL;
  mqtt_config.will_message_len =0;
  mqtt_config.will_retained = false;
  mqtt_config.will_qos =1;
  mqtt_config.username =USERNAME;
  if(USERNAME)
  {
	  mqtt_config.username_len =strlen(USERNAME);
	  mqtt_config.password =PASSWORD;
  }
  if(PASSWORD)
  {
  	  mqtt_config.password_len =strlen(PASSWORD);
  }
#ifdef MQTT_CLIENT_SECURITY
  memset(&(mqtt_config.ssl_cfg), 0, sizeof(mqtt_config.ssl_cfg)); 
  if( DEFAULT_SSL_ENABLE)
  {
    mqtt_config.ca_list = DEFAULT_LH_ROOT_CA_FILENAME;

    mqtt_config.cert = DEFAULT_LH_CERTIFICATE_FILENAME;

    mqtt_config.ssl_cfg.protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_2;

    mqtt_config.ssl_cfg.cipher[0] =
      QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384; /*still need to be
                                                        filled */
    mqtt_config.ssl_cfg.cipher[1] =
      QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384;
    mqtt_config.ssl_cfg.cipher[2] =
      QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256;
    mqtt_config.ssl_cfg.cipher[3] =
      QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256;
    mqtt_config.ssl_cfg.cipher[4] = QAPI_NET_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA;
    mqtt_config.ssl_cfg.cipher[5] = QAPI_NET_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA;
    mqtt_config.ssl_cfg.cipher[6] =
      QAPI_NET_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384;
    mqtt_config.ssl_cfg.cipher[7] =
      QAPI_NET_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384;

    mqtt_config.ssl_cfg.verify.domain = 0;
    mqtt_config.ssl_cfg.verify.time_Validity = 0;
    mqtt_config.ssl_cfg.verify.send_Alert = 0;
    mqtt_config.ssl_cfg.verify.match_Name[0] = '\0';

    /* Frangment value will be taken only in 512 , 1024 , 2048 , 4096   */
    mqtt_config.ssl_cfg.max_Frag_Len = 4096;
    mqtt_config.ssl_cfg.max_Frag_Len_Neg_Disable = 0;
  }

#endif
  return 0;
}

/**
 * @brief MQTT connection state callback.
 *
 * Invoked by the QAPI MQTT stack to notify about connection success/failure.
 *
 * @param[in] mqtt MQTT handle associated with the callback.
 * @param[in] reason Numeric reason code indicating connection result.
 */
static void mqtt_connect_callback(qapi_Net_MQTT_Hndl_t mqtt, int32_t reason)
{
	switch(reason)
	{
		case QAPI_NET_MQTT_CONNECT_SUCCEEDED_E:
			{      
				log_i("MQTT DAM: Connect Callback - Success");      
			}
		break;
			
		case QAPI_NET_MQTT_TCP_CONNECT_FAILED_E:
		case QAPI_NET_MQTT_SSL_CONNECT_FAILED_E:
		case QAPI_NET_MQTT_CONNECT_FAILED_E:
			{      
				log_e("MQTT DAM: Connect Callback - Failed with reason %d", reason);  
			}
		break;

		default:
				log_e("MQTT DAM: Connect Callback - Unknown reason %d", reason);
	}
}

/**
 * @brief MQTT subscribe / subscribe-message callback.
 *
 * Handles subscribe acknowledgements and incoming subscribe messages.
 *
 * @param[in] mqtt          MQTT handle.
 * @param[in] reason        Reason code for the subscribe event.
 * @param[in] topic         Topic buffer (not necessarily null-terminated).
 * @param[in] topic_length  Length of the topic buffer.
 * @param[in] qos           QoS associated with the subscription/event.
 * @param[in] sid           Optional session id pointer.
 */
static void mqtt_subscribe_callback(qapi_Net_MQTT_Hndl_t mqtt,
    int32_t reason,
    const uint8_t *topic,
    int32_t topic_length,
    int32_t qos,
    const void *sid)
{
    switch (reason)
    {
        case QAPI_NET_MQTT_SUBSCRIBE_GRANTED_E:
            log_i("MQTT DAM: Subscribe Granted");
            break;

        case QAPI_NET_MQTT_SUBSCRIBE_DENIED_E:
            log_e("MQTT DAM: Subscribe Denied");
            break;

        case QAPI_NET_MQTT_SUBSCRIBE_MSG_E:
            log_i("MQTT DAM: Subscribe Message");
            break;

        default:
            log_e("MQTT DAM: Subscribe Callback - Unknown reason %d", reason);
            break;
    }
}

/**
 * @brief MQTT message arrival callback.
 *
 * Invoked when a publish arrives on a subscribed topic.
 *
 * @param[in] mqtt MQTT handle.
 * @param[in] reason Event reason code.
 * @param[in] topic Topic name buffer (may not be null-terminated).
 * @param[in] topic_length Length of the topic buffer.
 * @param[in] msg Message payload buffer (not null-terminated).
 * @param[in] msg_length Length of the message payload.
 * @param[in] qos QoS of the message.
 * @param[in] sid Optional session id pointer.
 */
static void mqtt_message_callback(qapi_Net_MQTT_Hndl_t mqtt,
    int32_t reason,
    const uint8_t *topic,
    int32_t topic_length,
    const uint8_t *msg,
    int32_t msg_length,
    int32_t qos,
    const void *sid)
{ 
  char mqtt_msg[256]={0};
  	
  if(msg || topic)
	{
		strncpy(mqtt_msg,msg,msg_length);
		log_i("Message Callback: topic-%s qos-%d msg-\"%s\" reason-%d msg_length-%d", topic, qos , mqtt_msg, reason, msg_length);
	}
}

/**
 * @brief MQTT publish completion callback.
 *
 * Notifies about publish acknowledgement/state changes.
 *
 * @param[in] mqtt MQTT handle.
 * @param[in] msgtype Message type enum.
 * @param[in] qos QoS associated with the publish.
 * @param[in] msg_id Message identifier.
 */
static void mqtt_publish_callback(qapi_Net_MQTT_Hndl_t mqtt,
    enum QAPI_NET_MQTT_MSG_TYPES msgtype,
    int32_t qos,
    uint16_t msg_id)
{
	

  log_i("Publish Callback  QOS %d msg Id %d",qos, msg_id);
}

/**
  * @brief  MQTT init
  * @param  
  * @note   
  * @retval qapi_Status_t
  */
int mqtt_init() 
{
  qapi_Status_t  ret;

  if(is_mqtt_init)
    return 0;

  ret= qapi_Net_MQTT_New(&mqtt_handle);
  if(ret != QAPI_OK)
  {
    log_e("Context not created");
    return 1;
  }

  qapi_Net_MQTT_Pass_Pool_Ptr (mqtt_handle ,byte_pool_dam );

  qapi_Net_MQTT_Set_Connect_Callback(mqtt_handle,mqtt_connect_callback);
  qapi_Net_MQTT_Set_Subscribe_Callback(mqtt_handle,mqtt_subscribe_callback);
  qapi_Net_MQTT_Set_Message_Callback(mqtt_handle,mqtt_message_callback);
  qapi_Net_MQTT_Set_Publish_Callback(mqtt_handle,mqtt_publish_callback);

  ret =  qapi_Net_MQTT_Allow_Unsub_Publish(mqtt_handle,true);	 
  if (ret != QAPI_OK)	
	{	   
		log_e("qapi_Net_MQTT_Allow_Unsub_Publish returned error %d", ret);	   
		return 1;	  
	}

  
  is_mqtt_init=true; 

  return 0;
}

/**
  * @brief  MQTT deinit
  * @param  
  * @note   
  * @retval qapi_Status_t
  */
int mqtt_deinit() 
{
  if(is_mqtt_init) 
  {
    qapi_Net_MQTT_Destroy(mqtt_handle);
    is_mqtt_init=false;
  }
  return 0;
}

/**
  * @brief  MQTT connect
  * @param  remote_addr --broker address
  * @param  remote_port --broker port
  * @note   
  * @retval qapi_Status_t
  */
int mqtt_connect(char *remote_addr,int remote_port) 
{
  qapi_Status_t  ret=0;
  struct sockaddr_in *sin4=NULL;
  //Add by Jack,2020/12/14
  struct ip46addr  resolved_ip={0};
  char ip_str[16] = {0};
  if(false == is_mqtt_init && mqtt_init()) 
  {  
    log_e("mqtt can't init");
    return 1;
  }
  log_i("mqtt_connect addr[%s][%d]",remote_addr,remote_port);
  sin4 = (struct sockaddr_in *)&mqtt_config.local;
  sin4->sin_family = AF_INET;
  sin4->sin_port =htons(remote_port);
  sin4->sin_addr.s_addr = 0;

  sin4 = (struct sockaddr_in *)&mqtt_config.remote;
  sin4->sin_family = AF_INET;
  sin4->sin_port = htons(remote_port);

  if (inet_pton(AF_INET, remote_addr, &sin4->sin_addr.s_addr) == 1)
  {
      /* remote_addr is already a valid IPv4 address string */
      log_i("DAM_APP: Using IP address %s directly", remote_addr);
  }
  else
  {
      /* Not a valid IP — try DNS resolution */
      log_i("DAM_APP: Resolving hostname %s via DNS", remote_addr);
      resolved_ip.type = AF_INET;
      ret = qapi_Net_DNSc_Reshost(remote_addr, &resolved_ip);
      log_i("qapi_Net_DNSc_Reshost: 0x%x", ret);
      if (QAPI_DSS_SUCCESS == ret)
      {
          memcpy(&sin4->sin_addr.s_addr, &resolved_ip.a, 4);
          if (inet_ntop(AF_INET, &(resolved_ip.a), ip_str,
                        sizeof(CHAR) * IP_ADDR_SIZE))
          {
              log_i("DAM_APP: %s --> %s", remote_addr, ip_str);
          }
          else
          {
              log_i("DAM_APP: %s --> (inet_ntop failed)", remote_addr);
          }
      }
      else
      {
          log_e("DAM_APP: Failed to resolve %s for IPv4", remote_addr);
      }
  }

  ret = qapi_Net_MQTT_Connect(mqtt_handle, &mqtt_config);
  if(ret) 
  {
    log_e("MQTT Connect Error type %d", ret);
    log_e("MQTT Connect Failed");
  }
  else 
  {
    log_i("MQTT Connect Successfull");
  }
  return  ret;
}

/**
  * @brief  MQTT publish
  * @param  TOPIC --topic
  * @param  QOS --Qos
  * @param  RETAIN --retain setting
  * @param  MESSAGE --message pointer
  * @note   
  * @retval qapi_Status_t
  */
int mqtt_publish(uint8_t* TOPIC,uint8_t QOS,uint8_t RETAIN,uint8_t *MESSAGE)
{
  int  ret;
  uint8_t msg_len = 0;
  simcom_mqtt_pub_req req;

  if(false == is_mqtt_init && mqtt_init()) 
  {  
    log_e("mqtt can't init");
    return 1;
  }
  
  memset(&req,0,sizeof(simcom_mqtt_pub_req));  
  memcpy(req.Topic,TOPIC,strlen(TOPIC)+1);
  req.Qos=QOS; 
  req.Retain=RETAIN;
  snprintf((char *)req.Message,MESSAGE_MAX_LEN,"%s",MESSAGE);
  msg_len = strlen((const char *)req.Message);
  log_i("Publish DATA \"%s\"",(char *)req.Message);
  ret = qapi_Net_MQTT_Publish(mqtt_handle, (uint8_t *)req.Topic, (uint8_t *)req.Message,
      msg_len,req.Qos,req.Retain);
  if(ret) 
  {
    log_e("MQTT Publish Error type %d", ret);
  } 
  else
  {
    log_i("Published Successfull");
  }
  return ret;
}

/**
  * @brief  MQTT subscribe
  * @param  TOPIC --topic
  * @param  QOS --Qos
  * @note   
  * @retval qapi_Status_t
  */
int mqtt_subscribe(uint8_t* TOPIC,int32_t QOS)
{
  int ret;
  simcom_mqtt_sub_req req;
  if(false == is_mqtt_init && mqtt_init()) 
  {  
    log_e("mqtt can't init");
    return 1;
  }

  memset(&req,0,sizeof(simcom_mqtt_sub_req));
  memcpy(req.Topic,TOPIC,strlen(TOPIC)+1);
  req.Qos=QOS; 
  
  ret = qapi_Net_MQTT_Subscribe(mqtt_handle, (uint8_t *)req.Topic, QOS);
  if(ret) {
    log_e("MQTT Subscribe Error type %d", ret);
    log_e("MQTT Subscribe Failed");
  } else {
   log_i( "MQTT Subscribe Successfull");
  }
  return ret;
}

/**
  * @brief  MQTT disconnect
  * @param  
  * @note   
  * @retval 
  */
int mqtt_disconnect(void)
{
  qapi_Status_t  ret;
  if(false == is_mqtt_init) 
  {  
    return 0;
  }
  ret = qapi_Net_MQTT_Disconnect(mqtt_handle);
  if(ret)  
  {
    log_e("MQTT Disconnect Error type %d", ret);
  }
  else 
  {
    log_i("Disconnect Successfull");
  }
  return ret;
}
