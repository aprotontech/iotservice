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

#include "MQTTClient.h"

typedef enum _proton_mqtt_event_t {
  PME_DISCONNECT = 0,
  PME_NEW_MESSAGE,
  PME_MSG_DELIVERED,

  PME_EVENT_COUNT,
} proton_mqtt_event_t;

typedef struct _proton_mqtt_client_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  MQTTClient client;

  zval options;
  MQTTClient_connectOptions conn_opts;

  proton_work_t pw_connect;
  proton_work_t pw_disconnect;

  proton_private_value_t *status_channel;

  // subscribe topics(item: proton_mqtt_subscribe_topic_t)
  map_t subscribe_topics;

  // publish watcher(item: proton_mqtt_publish_watcher_t)
  map_t publish_watchers;
  pthread_mutex_t mwatcher;

  // message notify
  uv_async_t notify;
  pthread_mutex_t mevent;
  list_link_t msg_head;

} proton_mqtt_client_t;

typedef struct _proton_mqtt_subscribe_topic_t {
  char *topic;
  int topic_len;
  proton_private_value_t *channel;
} proton_mqtt_subscribe_topic_t;

typedef struct _proton_mqtt_publish_watcher_t {
  int dt;
  char *topic;
  MQTTClient_message message;
  proton_work_t pw_publish;
  proton_mqtt_client_t *mqtt;
  char dtid[20];
} proton_mqtt_publish_watcher_t;

int proton_mqttclient_init(proton_coroutine_runtime *runtime,
                           proton_mqtt_client_t *client, const char *client_id,
                           const char *host, int port);

int proton_mqttclient_uninit(proton_private_value_t *client);

int proton_mqttclient_connect(proton_private_value_t *value, zval *options,
                              proton_private_value_t *channel);

int proton_mqttclient_close(proton_private_value_t *client);

int proton_mqttclient_publish(proton_mqtt_client_t *mqtt, const char *topic,
                              int topic_len, const char *msg, int msg_len,
                              int qos, int retained, int *dt);

int proton_mqttclient_subscribe(proton_mqtt_client_t *mqtt, const char *topic,
                                int topic_len, int qos,
                                proton_private_value_t *channel);

#endif