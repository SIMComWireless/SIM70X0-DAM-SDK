/**
 * @file mqtt_demo.h
 * @brief Public MQTT demo declarations.
 *
 * Types and function prototypes used by the MQTT demo implementation.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __MQTT_DEMO_H__
#define __MQTT_DEMO_H__

#define  DEFAULT_MQTT_MAX_HOLD_TIME  (2*60)
#define  DEFAULT_SSL_ENABLE   0
#define DEFAULT_LH_ROOT_CA_FILENAME "ts_sdk_list.bin"
#define DEFAULT_LH_CERTIFICATE_FILENAME "ts_sdk_crt.bin"
#define APN_LEN                             20

//.....................................................................//
#define PRODUCT_KEY_MAX_LEN   				12
#define DEVICENAME_LEN						32
#define DEVICESECRET_LEN 					32
#define TOPIC_LEN							128
#define MESSAGE_MAX_LEN					    512  
#define MQTT_MESSAGE_MAX_LEN                2048 
#define IP_MAX_LEN							16 
                  
#pragma pack (1)
typedef struct MqttPubReqTag
{
	uint8_t Topic[TOPIC_LEN+1];
	uint8_t Qos;
	uint8_t Retain;
	uint8_t Message[MESSAGE_MAX_LEN+1];
}simcom_mqtt_pub_req;

typedef struct MqttSubReqTag
{
	uint8_t Topic[TOPIC_LEN+1];
	int Qos;
}simcom_mqtt_sub_req;

typedef struct MqttMsgTag
{
	uint8_t Topic[TOPIC_LEN+1];
	uint8_t Qos;
	uint8_t Message[MESSAGE_MAX_LEN+1];
}simcom_mqtt_msq_st;

//....................................................................//
int mqtt_config_set(char* CLIENT_ID,uint32_t KEEPALIVE_TIME,uint8_t CLEAN_SESSION,uint8_t* USERNAME, uint8_t* PASSWORD);
int mqtt_init(void);
int mqtt_deinit(void);
int mqtt_connect(char *remote_addr,int remote_port); 
int mqtt_publish(uint8_t* TOPIC,uint8_t QOS,uint8_t RETAIN,uint8_t *MESSAGE);
int mqtt_subscribe(uint8_t* TOPIC,int32_t QOS);
int mqtt_disconnect(void);

#endif


