/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     mqtt.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-09 11:56:31
 *
 */

#ifndef _PROTON_ELECTRON_MQTT_H_
#define _PROTON_ELECTRON_MQTT_H_

#include "proton/common/electron.h"
#include "proton/coroutine/runtime.h"
#include "proton/libuv/uvobject.h"
#include "proton/libuv/link_buff.h"
#include "proton/mqtt/third/mqtt.h"

#define MQTT_MESSAGE_MAX_LENGTH 20480

#define MAKESURE_STATUS_MATCH(cur_status, dest_status)                         \
  if ((cur_status) != (dest_status)) {                                         \
    PLOG_WARN("current status(%d) not match with dest status(%d)", cur_status, \
              dest_status);                                                    \
    return -1;                                                                 \
  }

typedef enum _proton_mqtt_event_t {
  PME_DISCONNECT = 0,
  PME_NEW_MESSAGE,
  PME_MSG_DELIVERED,

  PME_EVENT_COUNT,
} proton_mqtt_event_t;

typedef enum _proton_mqtt_client_status {
  MQTT_CLIENT_INITED = 1,
  MQTT_CLIENT_CONNECTING,
  MQTT_CLIENT_CONNECTED,
  MQTT_CLIENT_DISCONNECTED,
  MQTT_CLIENT_CONNECT_ERROR,
} proton_mqtt_client_status;

typedef struct _proton_mqtt_client_t proton_mqtt_client_t;
typedef int (*mqtt_client_subscribe_callback)(proton_mqtt_client_t *mqtt,
                                              zval *msg, zval *callback);

typedef struct _proton_mqtt_client_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  uv_tcp_t tcp;
  uv_timer_t timer;

  struct sockaddr_in server_addr;

  struct mqtt_client client;

  proton_mqtt_client_status status;

  proton_wait_object_t wq_connack;

  mqtt_client_subscribe_callback subscribe_callback;

  // subscribe topics(item: proton_mqtt_subscribe_topic_t)
  map_t subscribe_topics;

  // publish watcher(item: proton_mqtt_publish_watcher_t)
  map_t publish_watchers;
  pthread_mutex_t mwatcher;

  int is_looping;
  proton_wait_object_t wq_callback;
  list_link_t msg_head;

  int cur_recved_size;

} proton_mqtt_client_t;

typedef struct _proton_mqtt_subscribe_topic_t {
  char *topic;
  int topic_len;
  zval callback;
} proton_mqtt_subscribe_topic_t;

typedef struct _proton_mqtt_publish_watcher_t {
  int dt;
  char *topic;
  // MQTTClient_message message;
  proton_work_t pw_publish;
  proton_mqtt_client_t *mqtt;
  char dtid[20];
} proton_mqtt_publish_watcher_t;

typedef struct _proton_mqttclient_connect_options_t {
  char *client_id;
  char *user_name;
  char *password;
  char *will_topic;
  short keep_alive_interval;
  short clean_session;
  short reliable;
} proton_mqttclient_connect_options_t;

typedef struct _proton_mqttclient_callback_t {
  list_link_t link;
  char type; // 0-disconn,1-subscribe
  zval message;
} proton_mqttclient_callback_t;

int proton_mqttclient_init(proton_coroutine_runtime *runtime,
                           proton_mqtt_client_t *client, const char *host,
                           int port);

int proton_mqttclient_uninit(proton_private_value_t *client);

int proton_mqttclient_connect(proton_private_value_t *value,
                              proton_mqttclient_connect_options_t *option);

int proton_mqttclient_close(proton_private_value_t *client);

int proton_mqttclient_publish(proton_private_value_t *mqtt, const char *topic,
                              int topic_len, const char *msg, int msg_len,
                              int qos, int retained, int *dt);

int proton_mqttclient_subscribe(proton_private_value_t *mqtt, const char *topic,
                                int topic_len, int qos, zval *callback);

int proton_mqttclient_loop(proton_private_value_t *mqtt,
                           mqtt_client_subscribe_callback callback);

proton_mqtt_client_status
proton_mqttclient_get_status(proton_private_value_t *client);

//// internal functions
int _mqttclient_close_all(proton_mqtt_client_t *mqtt);
int parse_mqttclient_options(zval *options,
                             proton_mqttclient_connect_options_t *output);
int reset_mqtt_read_buffer(proton_mqtt_client_t *mqtt, int new_length);
void mqttclient_on_tick(uv_timer_t *timer);
void publish_callback(void **unused, struct mqtt_response_publish *published);
void mqttclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf);
void mqttclient_on_read(uv_stream_t *handle, ssize_t nread,
                        const uv_buf_t *buf);
void mqttclient_on_connected(uv_connect_t *req, int status);
#endif