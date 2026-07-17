/**
 * @file mqtt_demo.h
 * @brief MQTT client demo declarations.
 *
 * Type definitions, constants and function prototypes for the MQTT demo
 * implementation.  Provides simple connect / subscribe / publish helpers
 * wrapping the QAPI MQTT stack.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#ifndef __MQTT_DEMO_H__
#define __MQTT_DEMO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------
 * MQTT Configuration Constants
 *-----------------------------------------------------------------------*/
/** @brief Default MQTT keepalive time in seconds. */
#define DEFAULT_MQTT_MAX_HOLD_TIME      (2 * 60)

/** @brief Set to 1 to enable MQTT over TLS. */
#define DEFAULT_SSL_ENABLE              0

/** @brief Root CA filename for TLS. */
#define DEFAULT_LH_ROOT_CA_FILENAME     "ts_sdk_list.bin"

/** @brief Client certificate filename for TLS. */
#define DEFAULT_LH_CERTIFICATE_FILENAME "ts_sdk_crt.bin"

/** @brief Maximum APN string length. */
#define APN_LEN                         20

/** @brief Maximum product-key length. */
#define PRODUCT_KEY_MAX_LEN             12

/** @brief Maximum device-name length. */
#define DEVICENAME_LEN                  32

/** @brief Maximum device-secret length. */
#define DEVICESECRET_LEN                32

/** @brief Maximum topic string length. */
#define TOPIC_LEN                       128

/** @brief Maximum single message length. */
#define MESSAGE_MAX_LEN                 512

/** @brief Maximum MQTT message buffer length. */
#define MQTT_MESSAGE_MAX_LEN            2048

/** @brief Maximum IPv4 address string length. */
#define IP_MAX_LEN                      16

/*-------------------------------------------------------------------------
 * MQTT Request / Message Structures
 *-----------------------------------------------------------------------*/
#pragma pack(1)

/**
 * @brief MQTT publish request descriptor.
 */
typedef struct MqttPubReqTag
{
    uint8_t Topic[TOPIC_LEN + 1];      /**< Null-terminated topic string. */
    uint8_t Qos;                        /**< QoS level (0, 1, or 2). */
    uint8_t Retain;                     /**< Retain flag. */
    uint8_t Message[MESSAGE_MAX_LEN + 1]; /**< Null-terminated payload. */
} simcom_mqtt_pub_req;

/**
 * @brief MQTT subscribe request descriptor.
 */
typedef struct MqttSubReqTag
{
    uint8_t Topic[TOPIC_LEN + 1];      /**< Null-terminated topic string. */
    int     Qos;                        /**< Requested QoS level. */
} simcom_mqtt_sub_req;

/**
 * @brief MQTT received-message descriptor.
 */
typedef struct MqttMsgTag
{
    uint8_t Topic[TOPIC_LEN + 1];      /**< Null-terminated topic string. */
    uint8_t Qos;                        /**< QoS of the received message. */
    uint8_t Message[MESSAGE_MAX_LEN + 1]; /**< Null-terminated payload. */
} simcom_mqtt_msq_st;

#pragma pack()

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/**
 * @brief Configure MQTT connection parameters.
 *
 * @param[in] CLIENT_ID       Null-terminated client identifier.
 * @param[in] KEEPALIVE_TIME  Keepalive interval in seconds.
 * @param[in] CLEAN_SESSION   Non-zero for clean session.
 * @param[in] USERNAME        Optional username (may be NULL).
 * @param[in] PASSWORD        Optional password (may be NULL).
 *
 * @retval 0  Success.
 */
int mqtt_config_set(char *CLIENT_ID, uint32_t KEEPALIVE_TIME,
                     uint8_t CLEAN_SESSION, uint8_t *USERNAME,
                     uint8_t *PASSWORD);

/**
 * @brief Initialise the MQTT client handle and register callbacks.
 * @retval 0  Success.
 * @retval 1  Error.
 */
int mqtt_init(void);

/**
 * @brief Destroy the MQTT client handle and free resources.
 * @retval 0  Success.
 */
int mqtt_deinit(void);

/**
 * @brief Connect to an MQTT broker.
 *
 * Performs DNS resolution if @p remote_addr is a hostname.
 *
 * @param[in] remote_addr  Broker IP address or hostname.
 * @param[in] remote_port  Broker port number.
 *
 * @retval 0  Connected successfully.
 * @retval 1  Connection failed.
 */
int mqtt_connect(char *remote_addr, int remote_port);

/**
 * @brief Publish a message to a topic.
 *
 * @param[in] TOPIC    Null-terminated topic string.
 * @param[in] QOS      QoS level (0, 1, or 2).
 * @param[in] RETAIN   Retain flag.
 * @param[in] MESSAGE  Null-terminated payload string.
 *
 * @retval 0  Published successfully.
 */
int mqtt_publish(uint8_t *TOPIC, uint8_t QOS, uint8_t RETAIN,
                  uint8_t *MESSAGE);

/**
 * @brief Subscribe to a topic.
 *
 * @param[in] TOPIC  Null-terminated topic string.
 * @param[in] QOS    Requested QoS level.
 *
 * @retval 0  Subscribed successfully.
 */
int mqtt_subscribe(uint8_t *TOPIC, int32_t QOS);

/**
 * @brief Disconnect from the MQTT broker.
 * @retval 0  Disconnected successfully.
 */
int mqtt_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* __MQTT_DEMO_H__ */
