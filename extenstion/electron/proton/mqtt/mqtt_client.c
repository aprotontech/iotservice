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

#define GET_LONG_OPTION(dest, source, name, must_exists)                       \
  long dest = 0;                                                               \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest = Z_LVAL_P(val);                                                    \
    } else if (must_exists) {                                                  \
      PLOG_WARN("key %s not found in option", #name);                          \
      return -1;                                                               \
    }                                                                          \
  }

#define GET_STRING_OPTION(dest, source, name, must_exists)                     \
  char *dest = NULL;                                                           \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest = ZSTR_VAL(Z_STR_P(val));                                           \
    } else if (must_exists) {                                                  \
      PLOG_WARN("key %s not found in option", #name);                          \
      return -1;                                                               \
    }                                                                          \
  }

int reset_mqtt_read_buffer(proton_mqtt_client_t *mqtt, int new_length);
void mqttclient_on_tick(uv_timer_t *timer);
void publish_callback(void **unused, struct mqtt_response_publish *published);
void mqttclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf);
void mqttclient_on_read(uv_stream_t *handle, ssize_t nread,
                        const uv_buf_t *buf);
void mqttclient_on_connected(uv_connect_t *req, int status);

int proton_mqttclient_subscribe(proton_private_value_t *value,
                                const char *topic, int topic_len, int qos,
                                proton_private_value_t *channel) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(topic);
  MAKESURE_PTR_NOT_NULL(channel);
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

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

  if (0 == rc) {
    proton_mqtt_subscribe_topic_t *st =
        (proton_mqtt_subscribe_topic_t *)qmalloc(
            sizeof(proton_mqtt_subscribe_topic_t) + topic_len + 1);
    st->channel = channel;
    st->topic = (char *)(&st[1]);
    st->topic_len = topic_len;
    strcpy(st->topic, topic);
    Z_TRY_ADDREF(channel->myself);

    hashmap_put(mqtt->subscribe_topics, st->topic, (any_t)st);
  }

  return rc;
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

int proton_mqttclient_connect(proton_private_value_t *value, zval *options,
                              proton_private_value_t *channel) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(options);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;

  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (IS_COROUTINE_WAITFOR(mqtt->wq_connack)) {
    PLOG_WARN("[MQTT] mqtt client is connecting...");
    return -1;
  }

  GET_STRING_OPTION(_client_id, options, clientId, 1);
  GET_STRING_OPTION(_user_name, options, userName, 0);
  GET_STRING_OPTION(_password, options, password, 1);
  GET_STRING_OPTION(_will_topic, options, willTopic, 0);
  GET_LONG_OPTION(_kl_second, options, keepAliveInterval, 1);
  GET_LONG_OPTION(_clean_session, options, cleanSession, 0);
  GET_LONG_OPTION(_reliable, options, reliable, 0);

  // FIRST: tcp connect to the server
  proton_connect_t pc = {
      .status = 0,
  };
  PROTON_WAIT_OBJECT_INIT(pc.wq_connect);
  pc.connect.data = &pc;

  struct sockaddr_in addr;
  uv_ip4_addr(mqtt->host, mqtt->port, &addr);

  int rc = uv_tcp_connect(&pc.connect, &mqtt->tcp, (struct sockaddr *)&addr,
                          mqttclient_on_connected);
  if (rc != 0) {
    PLOG_WARN("mqtt tcp connect failed with %d", rc);
    return -1;
  }

  rc = proton_coroutine_waitfor(mqtt->runtime, &pc.wq_connect, NULL);
  if (rc != 0) {
    return rc;
  }

  // start reading
  uv_read_start((uv_stream_t *)(&mqtt->tcp), mqttclient_alloc_buffer,
                mqttclient_on_read);

  // SECOND: send connect message to server
  uint8_t connect_flags = 0;
  if (_clean_session) {
    connect_flags |= MQTT_CONNECT_CLEAN_SESSION;
  }
  if (_user_name == NULL) {
    _user_name = _client_id;
  }

  mqtt_connect(&mqtt->client, _client_id, _will_topic, NULL, 0, _user_name,
               _password, connect_flags, _kl_second);
  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("package mqtt connect message failed");
    return RC_ERROR_MQTT_CONNECT;
  }

  __mqtt_send(&mqtt->client);
  if (mqtt->client.error != MQTT_OK) {
    PLOG_WARN("send mqtt connect message failed");
    return RC_ERROR_MQTT_CONNECT;
  }

  // THIRD: wait for mqtt CONNACK response
  rc = proton_coroutine_waitfor(mqtt->runtime, &mqtt->wq_connack, NULL);
  if (rc == 0) {
    if (mqtt->client.error != MQTT_OK) { // connect failed
      PLOG_WARN("mqtt connect failed with %d", mqtt->client.error);
      return RC_ERROR_MQTT_CONNECT;
    }

    uv_timer_start(&mqtt->timer, mqttclient_on_tick, 1000, 1000);

    mqtt->status_channel = channel;
    if (channel != NULL) { // add ref
      Z_TRY_ADDREF(channel->myself);
    }
  }

  return rc;
}

int proton_mqttclient_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

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

  struct sockaddr_in addr;
  int rc = uv_ip4_addr(host, port, &addr);
  if (rc != 0) {
    PLOG_WARN("parse ip(%s:%d) failed.", host, port);
    return -1;
  }

  proton_link_buffer_init(&mqtt->buffers, 4096, 40960);
  proton_buffer_t *sendbuf = proton_link_buffer_new_slice(&mqtt->buffers, 4096);
  proton_buffer_t *recvbuf = proton_link_buffer_new_slice(&mqtt->buffers, 1024);

  sendbuf->used = sendbuf->buff.len;
  recvbuf->used = recvbuf->buff.len;

  if (mqtt_init(&mqtt->client, mqtt, sendbuf->buff.base, sendbuf->buff.len,
                recvbuf->buff.base, recvbuf->buff.len,
                publish_callback) != MQTT_OK) {
    PLOG_WARN("init mqtt client failed");
    proton_link_buffer_uninit(&mqtt->buffers);
    return -1;
  }

  mqtt->read_buffer = NULL;
  reset_mqtt_read_buffer(mqtt, 2048);

  mqtt->host =
      proton_link_buffer_copy_string(&mqtt->buffers, host, strlen(host));
  mqtt->port = port;

  pthread_mutex_init(&mqtt->mevent, NULL);
  pthread_mutex_init(&mqtt->mwatcher, NULL);

  mqtt->status_channel = NULL;
  mqtt->subscribe_topics = hashmap_new();
  mqtt->publish_watchers = hashmap_new();

  PLOG_DEBUG("mqtt adress(%s:%d)", host, port);

  uv_timer_init(RUNTIME_UV_LOOP(runtime), &mqtt->timer);
  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &mqtt->tcp);
  mqtt->tcp.data = mqtt;
  mqtt->timer.data = mqtt;

  mqtt->notify.data = mqtt;

  /*rc = uv_async_init(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->notify,
                     mqtt_notify_to_php);
  if (rc != 0) {
    PLOG_WARN("mqtt(%p) uv_async_init failed with %d", mqtt, rc);
    return -1;
  }*/

  return 0;
}

static int _free_stopic_item(any_t item, const char *key, any_t data);
static int _free_pwatcher_item(any_t item, const char *key, any_t data);
int proton_mqttclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  uv_close((uv_handle_t *)&mqtt->notify, NULL);

  if (IS_COROUTINE_WAITFOR(mqtt->wq_connack)) {
    proton_coroutine_cancel(mqtt->runtime, &mqtt->wq_connack, NULL);
  }

  if (mqtt->status_channel != NULL) {
    RELEASE_VALUE_MYSELF(*mqtt->status_channel);
    mqtt->status_channel = NULL;
  }

  if (mqtt->subscribe_topics != NULL) {
    hashmap_iterate(mqtt->subscribe_topics, _free_stopic_item, NULL);
    hashmap_free(mqtt->subscribe_topics);
  }

  if (mqtt->publish_watchers != NULL) {
    hashmap_iterate(mqtt->publish_watchers, _free_pwatcher_item, NULL);
    hashmap_free(mqtt->publish_watchers);
  }

  proton_link_buffer_uninit(&mqtt->buffers);

  if (mqtt->read_buffer != NULL) {
    free(mqtt->read_buffer);
  }

  return 0;
}

static int _free_stopic_item(any_t item, const char *key, any_t data) {
  proton_mqtt_subscribe_topic_t *topic = (proton_mqtt_subscribe_topic_t *)item;
  if (topic->channel != NULL) {
    RELEASE_VALUE_MYSELF(*topic->channel);
  }
  qfree(topic);
  return MAP_OK;
}

static int _free_pwatcher_item(any_t item, const char *key, any_t data) {
  return MAP_OK;
}