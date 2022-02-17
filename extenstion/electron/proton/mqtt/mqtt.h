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

  PME_EVENT_COUNT,
} proton_mqtt_event_t;

typedef int (*mqttclient_on_event)(proton_mqtt_event_t event_type,
                                   const char *topic, const char *payload,
                                   int msgid, int qos, int retained, int dup);

typedef struct _proton_mqtt_client_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  MQTTClient client;

  zval options;
  MQTTClient_connectOptions conn_opts;

  proton_work_t connect_work;
  proton_work_t disconnect_work;

  // message notify
  uv_async_t notify;
  list_link_t msg_head;
  mqttclient_on_event events[PME_EVENT_COUNT];

  pthread_mutex_t mevent;

} proton_mqtt_client_t;

int proton_mqttclient_init(proton_coroutine_runtime *runtime,
                           proton_mqtt_client_t *client, const char *client_id,
                           const char *host, int port);

int proton_mqttclient_uninit(proton_private_value_t *client);

int proton_mqttclient_connect(proton_private_value_t *value, zval *options);

int proton_mqttclient_close(proton_private_value_t *client);

#endif