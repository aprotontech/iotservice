/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     mqtt.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-09 11:56:57
 *
 */

#include "mqtt_client.h"

PROTON_TYPE_WHOAMI_DEFINE(_mqtt_client_get_type, "mqttclient")

static proton_value_type_t __proton_mqttclient_type = {
    .construct = NULL,
    .destruct = proton_mqttclient_uninit,
    .whoami = _mqtt_client_get_type};

void _update_mqttclient_status(proton_mqtt_client_t *mqtt, int status,
                               const char *msg) {
  PLOG_DEBUG("mqtt(%p) update status to %d, msg(%s)", mqtt, status, msg);
  mqtt->status = status;
  if (mqtt->is_looping) {
    proton_mqttclient_callback_t *cbmsg =
        (proton_mqttclient_callback_t *)qmalloc(
            sizeof(proton_mqttclient_callback_t));
    cbmsg->type = 0;
    msg = (msg != NULL) ? msg : "";
    ZVAL_STRINGL(&cbmsg->message, msg, strlen(msg));

    LL_insert(&cbmsg->link, mqtt->msg_head.prev);

    proton_coroutine_wakeup(mqtt->runtime, &mqtt->wq_callback, NULL);
  }
}

int proton_mqttclient_subscribe(proton_private_value_t *value,
                                const char *topic, int topic_len, int qos,
                                proton_private_value_t *channel) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(topic);
  MAKESURE_PTR_NOT_NULL(channel);
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (mqtt->status != MQTT_CLIENT_CONNECTED || !mqtt->is_looping) {
    PLOG_WARN("[MQTT] mqtt client must connected and loop before publish");
    return -1;
  }

  any_t topic_item = NULL;
  if (hashmap_get(mqtt->subscribe_topics, (char *)topic, &topic_item) ==
      MAP_OK) { // exists subscribe
    PLOG_WARN("mqtt(%p) had subscribe topic(%s)", mqtt, topic);
    return -1;
  }

  PLOG_INFO("mqtt(%p) subcribe(%s), qos(%d)", mqtt, topic, qos);
  mqtt_subscribe(&mqtt->client, topic, qos);

  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("package mqtt subscribe message failed");
    return RC_ERROR_MQTT_SUBSCRIBE;
  }

  __mqtt_send(&mqtt->client);
  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("send mqtt subscribe message failed");
    return RC_ERROR_MQTT_SUBSCRIBE;
  }

  proton_mqtt_subscribe_topic_t *st = (proton_mqtt_subscribe_topic_t *)qmalloc(
      sizeof(proton_mqtt_subscribe_topic_t) + topic_len + 1);
  st->topic = (char *)(&st[1]);
  st->topic_len = topic_len;
  strcpy(st->topic, topic);
  Z_TRY_ADDREF(channel->myself);

  hashmap_put(mqtt->subscribe_topics, st->topic, (any_t)st);

  return 0;
}

// publish
static void mqtt_publish_task(uv_work_t *req) {
  proton_mqtt_publish_watcher_t *pw =
      (proton_mqtt_publish_watcher_t *)req->data;

  int token;
  // MQTTClient_deliveryToken token;
  int rc = 0;
  // MQTTClient_publishMessage(pw->mqtt->client, pw->topic,
  // &pw->message,&token);
  if (rc == 0) {
    snprintf(pw->dtid, sizeof(pw->dtid), "%d", (int)token);
    pthread_mutex_lock(&pw->mqtt->mwatcher);

    hashmap_put(pw->mqtt->publish_watchers, pw->dtid, pw);

    pthread_mutex_unlock(&pw->mqtt->mwatcher);

    if (pw->pw_publish.result != NULL) {
      *((int *)pw->pw_publish.result) = (int)token;
    }
  }

  PLOG_INFO("mqtt(%p) publish(%s), ret(%d)", pw->mqtt, pw->topic, rc);
}

static void mqtt_publish_result_callback(uv_work_t *req, int status) {

  proton_mqtt_publish_watcher_t *pw =
      (proton_mqtt_publish_watcher_t *)req->data;
  pw->pw_publish.status = status;

  // do nothing. wait for dt callback
}

int proton_mqttclient_publish(proton_private_value_t *value, const char *topic,
                              int topic_len, const char *msg, int msg_len,
                              int qos, int retained, int *dt) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(topic);
  MAKESURE_PTR_NOT_NULL(msg);
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (mqtt->status != MQTT_CLIENT_CONNECTED || !mqtt->is_looping) {
    PLOG_WARN("[MQTT] mqtt client must connected and loop before publish");
    return -1;
  }

  proton_mqtt_publish_watcher_t pwmsg;
  proton_mqtt_publish_watcher_t *pw = &pwmsg;
  memset(pw, 0, sizeof(proton_mqtt_publish_watcher_t));

  /*
    pw->message.dup = 0;
    pw->message.qos = qos;
    pw->message.retained = retained;
    pw->message.payload = (void *)msg;
    pw->message.payloadlen = msg_len;*/
  pw->topic = (char *)topic;

  pw->mqtt = mqtt;

  pw->pw_publish.work.data = pw;
  pw->pw_publish.status = 0;
  pw->pw_publish.result = dt;
  PROTON_WAIT_OBJECT_INIT(pw->pw_publish.wq_work);

  int rc = uv_queue_work(RUNTIME_UV_LOOP(mqtt->runtime), &pw->pw_publish.work,
                         mqtt_publish_task, mqtt_publish_result_callback);
  if (0 != rc) {
    PLOG_WARN("mqtt connect uv_queue_work failed with %d", rc);
    return rc;
  }

  if ((rc = proton_coroutine_waitfor(mqtt->runtime, &pw->pw_publish.wq_work,
                                     NULL)) != 0) {
    // make sure watcher is in map
    if (strlen(pw->dtid) != 0) {
      pthread_mutex_lock(&mqtt->mwatcher);
      hashmap_remove(mqtt->publish_watchers, pw->dtid);
      pthread_mutex_unlock(&mqtt->mwatcher);
    }

    return rc;
  }

  if (pw->pw_publish.status != 0) {
    return -1;
  }

  return 0;
}

///////////////// MQTT CONNECT/DISCONNECT
void mqttclient_alloc_buffer_conncb(uv_handle_t *handle, size_t suggested_size,
                                    uv_buf_t *buf) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  // CONNACK size is 4bytes
  *buf = uv_buf_init(mqtt->client.recv_buffer.mem_start, 4);
}
void mqttclient_on_read_conncb(uv_stream_t *handle, ssize_t nread,
                               const uv_buf_t *buf) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  if (nread == 4) {
    mqtt->cur_recved_size = nread;
    mqtt_sync(&mqtt->client);

    PLOG_DEBUG("mqtt(%p) read connack", mqtt);

    // stop read new data
    uv_read_stop((uv_stream_t *)(&mqtt->tcp));
  } else {
    _mqttclient_close_all(mqtt);
  }

  proton_coroutine_wakeup(mqtt->runtime, &mqtt->wq_connack, NULL);
}

int _mqttclient_connect_to_remote(
    proton_mqtt_client_t *mqtt, proton_mqttclient_connect_options_t *options) {
  // FIRST: tcp connect to the server
  proton_connect_t pc = {
      .status = 0,
  };
  PROTON_WAIT_OBJECT_INIT(pc.wq_connect);
  pc.connect.data = &pc;

  int rc = uv_tcp_connect(&pc.connect, &mqtt->tcp,
                          (struct sockaddr *)&mqtt->server_addr,
                          mqttclient_on_connected);
  if (rc != 0) {
    mqtt->status = MQTT_CLIENT_CONNECT_ERROR;
    PLOG_WARN("mqtt(%p) tcp connect failed with %d, err=%s", mqtt, rc,
              uv_err_name(rc));
    return RC_ERROR_MQTT_CONNECT;
  }

  RC_EXPECT_SUCCESS(
      proton_coroutine_waitfor(mqtt->runtime, &pc.wq_connect, NULL));
  RC_EXPECT_SUCCESS_WITH_EMSG(pc.status, "connect failed");

  // start reading
  uv_read_start((uv_stream_t *)(&mqtt->tcp), mqttclient_alloc_buffer_conncb,
                mqttclient_on_read_conncb);

  // SECOND: send connect message to server
  uint8_t connect_flags = 0;
  if (options->clean_session) {
    connect_flags |= MQTT_CONNECT_CLEAN_SESSION;
  }

  mqtt_connect(&mqtt->client, options->client_id, options->will_topic, NULL, 0,
               options->user_name, options->password, connect_flags,
               options->keep_alive_interval);
  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("package mqtt connect message failed");
    return RC_ERROR_MQTT_CONNECT;
  }

  __mqtt_send(&mqtt->client);
  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("send mqtt connect message failed");
    _mqttclient_close_all(mqtt);
    return RC_ERROR_MQTT_CONNECT;
  }

  // THIRD: wait for mqtt CONNACK response
  RC_EXPECT_SUCCESS(
      proton_coroutine_waitfor(mqtt->runtime, &mqtt->wq_connack, NULL));

  if (mqtt->client.error != MQTT_OK) { // connect failed
    PLOG_WARN("mqtt(%p) connect failed with %d, err=%s", mqtt,
              mqtt->client.error, mqtt_error_str(mqtt->client.error));
    _mqttclient_close_all(mqtt);
    return RC_ERROR_MQTT_CONNECT;
  }

  return 0;
}

int proton_mqttclient_connect(proton_private_value_t *value,
                              proton_mqttclient_connect_options_t *options,
                              mqtt_client_status_changed callback) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(options);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;

  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (mqtt->status == MQTT_CLIENT_CONNECTING ||
      mqtt->status == MQTT_CLIENT_CONNECTED) {
    PLOG_WARN("[MQTT] mqtt client status %d is error", mqtt->status);
    return -1;
  }

  if (IS_COROUTINE_WAITFOR(mqtt->wq_connack)) {
    PLOG_WARN("[MQTT] mqtt client is connecting...");
    return -1;
  }

  if (mqtt->is_looping) {
    PLOG_WARN("[MQTT] mqtt client must stop loop before connect");
    return -1;
  }

  mqtt->status_callback = callback;
  _update_mqttclient_status(mqtt, MQTT_CLIENT_CONNECTING, "connecting");

  if (_mqttclient_connect_to_remote(mqtt, options) != 0) {
    _update_mqttclient_status(mqtt, MQTT_CLIENT_CONNECT_ERROR, "error");
    mqtt->status_callback = NULL;
    return -1;
  } else {
    _update_mqttclient_status(mqtt, MQTT_CLIENT_CONNECTED, "connected");
  }

  return 0;
}

int proton_mqttclient_loop(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (mqtt->is_looping) {
    PLOG_WARN("some coroutine is loop, can't loop again");
    return -1;
  }

  if (mqtt->status != MQTT_CLIENT_CONNECTED) {
    PLOG_WARN("[MQTT] mqtt client must connected before loop");
    return -1;
  }

  mqtt->is_looping = 1;
  // resume read data
  uv_read_start((uv_stream_t *)(&mqtt->tcp), mqttclient_alloc_buffer,
                mqttclient_on_read);
  uv_timer_start(&mqtt->timer, mqttclient_on_tick, 1000, 1000);

  do { // check has msg callback
    list_link_t *p = mqtt->msg_head.next;
    while (p != &mqtt->msg_head) {
      proton_mqttclient_callback_t *msg =
          container_of(p, proton_mqttclient_callback_t, link);
      p = p->next;
      switch (msg->type) {
      case -1:
        break;
      case 0:
        // status callback
        if (mqtt->status_callback != NULL) {
          mqtt->status_callback(mqtt, mqtt->status, &msg->message);
        }
        break;
      case 1:
      default:
        // subscribe callback
        if (mqtt->subscribe_callback != NULL) {
          zval callback;
          mqtt->subscribe_callback(mqtt, &msg->message, &callback);
        }
        break;
      }

      qfree(msg);
    }
  } while (proton_coroutine_waitfor(mqtt->runtime, &mqtt->wq_callback, NULL) ==
           0);
}

proton_mqtt_client_status
proton_mqttclient_get_status(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  return mqtt->status;
}

int proton_mqttclient_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (mqtt->status == MQTT_CLIENT_CONNECTED) {
    mqtt_disconnect(&mqtt->client);
    if (mqtt->client.error != MQTT_OK) {
      PLOG_WARN("package mqtt disconnect message failed");
      return -1;
    }

    __mqtt_send(&mqtt->client);
    if (mqtt->client.error != MQTT_OK) {
      PLOG_WARN("send mqtt disconnect message failed");
      return -1;
    }

    mqtt->status = MQTT_CLIENT_DISCONNECTED;
  }

  _mqttclient_close_all(mqtt);

  return 0;
}

//////////////////////// MQTT INIT/UNINIT
int proton_mqttclient_init(proton_coroutine_runtime *runtime,
                           proton_mqtt_client_t *mqtt, const char *host,
                           int port) {

  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(mqtt);
  MAKESURE_PTR_NOT_NULL(host);

  memset(mqtt, 0, sizeof(proton_mqtt_client_t));

  mqtt->runtime = runtime;
  mqtt->value.type = &__proton_mqttclient_type;
  ZVAL_UNDEF(&mqtt->value.myself);

  PROTON_WAIT_OBJECT_INIT(mqtt->wq_connack);

  PLOG_DEBUG("mqtt adress(%s:%d)", host, port);
  int rc = uv_ip4_addr(host, port, &mqtt->server_addr);
  if (rc != 0) {
    PLOG_WARN("parse ip(%s:%d) failed.", host, port);
    return -1;
  }

  const int default_buf_len = 4096;
  char *recvbuf = (char *)qmalloc(default_buf_len);
  char *sendbuf = (char *)qmalloc(default_buf_len);

  if (mqtt_init(&mqtt->client, mqtt, sendbuf, default_buf_len, recvbuf,
                default_buf_len, publish_callback) != MQTT_OK) {
    PLOG_WARN("init mqtt client failed");
    return -1;
  }

  pthread_mutex_init(&mqtt->mwatcher, NULL);

  mqtt->status_callback = NULL;

  mqtt->subscribe_topics = hashmap_new();
  mqtt->publish_watchers = hashmap_new();

  uv_timer_init(RUNTIME_UV_LOOP(runtime), &mqtt->timer);
  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &mqtt->tcp);
  mqtt->tcp.data = mqtt;
  mqtt->timer.data = mqtt;

  mqtt->is_looping = 0;

  return 0;
}

static int _free_stopic_item(any_t item, const char *key, any_t data);
static int _free_pwatcher_item(any_t item, const char *key, any_t data);
int proton_mqttclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;

  if (IS_COROUTINE_WAITFOR(mqtt->wq_connack)) {
    proton_coroutine_cancel(mqtt->runtime, &mqtt->wq_connack, NULL);
  }

  if (mqtt->subscribe_topics != NULL) {
    hashmap_iterate(mqtt->subscribe_topics, _free_stopic_item, NULL);
    hashmap_free(mqtt->subscribe_topics);
  }

  if (mqtt->publish_watchers != NULL) {
    hashmap_iterate(mqtt->publish_watchers, _free_pwatcher_item, NULL);
    hashmap_free(mqtt->publish_watchers);
  }

  return 0;
}

static int _free_stopic_item(any_t item, const char *key, any_t data) {
  proton_mqtt_subscribe_topic_t *topic = (proton_mqtt_subscribe_topic_t *)item;

  qfree(topic);
  return MAP_OK;
}

static int _free_pwatcher_item(any_t item, const char *key, any_t data) {
  return MAP_OK;
}