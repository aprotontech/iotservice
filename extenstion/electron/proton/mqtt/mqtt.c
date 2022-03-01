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

#include "mqtt.h"

PROTON_TYPE_WHOAMI_DEFINE(_mqtt_client_get_type, "mqttclient")

static proton_value_type_t __proton_mqttclient_type = {
    .construct = NULL,
    .destruct = proton_mqttclient_uninit,
    .whoami = _mqtt_client_get_type};

#define FILL_VALUE_LONG(dest, source, name)                                    \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest.name = Z_LVAL_P(val);                                               \
    }                                                                          \
  }

#define FILL_VALUE_STRING(dest, source, name)                                  \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest.name = ZSTR_VAL(Z_STR_P(val));                                      \
    }                                                                          \
  }

extern void mqtt_notify_to_php(uv_async_t *req);
extern void connlost(void *context, char *cause);
extern void delivered(void *context, MQTTClient_deliveryToken dt);
extern int msgarrvd(void *context, char *topicName, int topicLen,
                    MQTTClient_message *message);

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
  int rc = MQTTClient_subscribe(mqtt->client, topic, qos);
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

  MQTTClient_deliveryToken token;
  int rc = MQTTClient_publishMessage(pw->mqtt->client, pw->topic, &pw->message,
                                     &token);
  if (rc == MQTTCLIENT_SUCCESS) {
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

  pw->message.dup = 0;
  pw->message.qos = qos;
  pw->message.retained = retained;
  pw->message.payload = (void *)msg;
  pw->message.payloadlen = msg_len;
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
static void mqtt_connect_task(uv_work_t *req) {
  int rc;
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  if ((rc = MQTTClient_connect(mqtt->client, &mqtt->conn_opts)) !=
      MQTTCLIENT_SUCCESS) {
    PLOG_WARN("mqtt(%p) Failed to connect, return code %d", mqtt, rc);
  }

  if (mqtt->pw_connect.result != NULL) {
    *((int *)mqtt->pw_connect.result) = rc;
  }
}

static void mqtt_connect_result_callback(uv_work_t *req, int status) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;

  proton_coroutine_wakeup(mqtt->runtime, &mqtt->pw_connect.wq_work, NULL);
}

static void mqtt_disconnect_task(uv_work_t *req) {

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  int rc = MQTTClient_disconnect(mqtt->client, 5);
  if (rc != MQTTCLIENT_SUCCESS) {
    PLOG_WARN("mqtt(%p) Failed to disconnect, return code %d", mqtt, rc);
  }

  if (mqtt->pw_disconnect.result != NULL) {
    *((int *)mqtt->pw_disconnect.result) = rc;
  }
}

static void mqtt_disconnect_result(uv_work_t *req, int status) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;

  PLOG_DEBUG("mqtt(%p) mqtt_disconnect_result(%d)", mqtt, status);
  mqtt->pw_disconnect.status = status;
  proton_coroutine_wakeup(mqtt->runtime, &mqtt->pw_disconnect.wq_work, NULL);
}

int proton_mqttclient_connect(proton_private_value_t *value, zval *options,
                              proton_private_value_t *channel) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(options);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;

  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (!LL_isspin(&mqtt->pw_connect.wq_work.head)) {
    PLOG_WARN("[MQTT] mqtt client is connecting...");
    return -1;
  }

  ZVAL_COPY(&mqtt->options, options);
  options = &mqtt->options;
  FILL_VALUE_LONG(mqtt->conn_opts, options, keepAliveInterval);
  FILL_VALUE_LONG(mqtt->conn_opts, options, cleansession);
  FILL_VALUE_LONG(mqtt->conn_opts, options, reliable);
  FILL_VALUE_LONG(mqtt->conn_opts, options, connectTimeout);
  FILL_VALUE_LONG(mqtt->conn_opts, options, retryInterval);
  FILL_VALUE_STRING(mqtt->conn_opts, options, username);
  FILL_VALUE_STRING(mqtt->conn_opts, options, password);

  int connect_result = -1;
  mqtt->pw_connect.result = &connect_result;

  int rc = uv_queue_work(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->pw_connect.work,
                         mqtt_connect_task, mqtt_connect_result_callback);
  if (0 != rc) {
    PLOG_WARN("mqtt connect uv_queue_work failed with %d", rc);
    return rc;
  }

  if ((rc = proton_coroutine_waitfor(mqtt->runtime, &mqtt->pw_connect.wq_work,
                                     NULL)) != 0) {
    return rc;
  }

  if (mqtt->pw_connect.status != 0) {
    return -1;
  }

  if (rc == 0) {
    mqtt->status_channel = channel;
    if (channel != NULL) { // add ref
      Z_TRY_ADDREF(channel->myself);
    }
  }

  return connect_result;
}

int proton_mqttclient_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (!LL_isspin(&mqtt->pw_disconnect.wq_work.head)) {
    PLOG_WARN("[MQTT] mqtt client is disconnecting...");
    return -1;
  }

  int disconnect_result = -1;
  mqtt->pw_disconnect.result = &disconnect_result;

  int rc =
      uv_queue_work(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->pw_disconnect.work,
                    mqtt_disconnect_task, mqtt_disconnect_result);
  if (0 != rc) {
    PLOG_WARN("mqtt disconnect uv_queue_work failed with %d", rc);
    return rc;
  }

  if ((rc = proton_coroutine_waitfor(
           mqtt->runtime, &mqtt->pw_disconnect.wq_work, NULL)) != 0) {
    return rc;
  }

  if (mqtt->pw_disconnect.status != 0) {
    return -1;
  }

  if (mqtt->status_channel != NULL) {
    RELEASE_VALUE_MYSELF(*mqtt->status_channel);
    mqtt->status_channel = NULL;
  }

  return disconnect_result;
}

//////////////////////// MQTT INIT/UNINIT
int proton_mqttclient_init(proton_coroutine_runtime *runtime,
                           proton_mqtt_client_t *mqtt, const char *client_id,
                           const char *host, int port) {

  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(mqtt);
  MAKESURE_PTR_NOT_NULL(host);

  memset(mqtt, 0, sizeof(proton_mqtt_client_t));

  mqtt->runtime = runtime;
  mqtt->value.type = &__proton_mqttclient_type;
  ZVAL_UNDEF(&mqtt->value.myself);
  ZVAL_UNDEF(&mqtt->options);

  mqtt->pw_connect.work.data = mqtt;
  mqtt->pw_connect.status = 0;
  mqtt->pw_connect.result = NULL;
  PROTON_WAIT_OBJECT_INIT(mqtt->pw_connect.wq_work);
  mqtt->pw_disconnect.work.data = mqtt;
  mqtt->pw_disconnect.status = 0;
  mqtt->pw_disconnect.result = NULL;
  PROTON_WAIT_OBJECT_INIT(mqtt->pw_disconnect.wq_work);

  pthread_mutex_init(&mqtt->mevent, NULL);
  pthread_mutex_init(&mqtt->mwatcher, NULL);

  mqtt->status_channel = NULL;
  mqtt->subscribe_topics = hashmap_new();
  mqtt->publish_watchers = hashmap_new();

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

  char address[256];

  if (snprintf(address, sizeof(address), "%s:%d", host, port) >=
      sizeof(address)) {
    PLOG_WARN("input adress(%s:%d) is too long", host, port);
    return -1;
  }

  PLOG_DEBUG("mqtt adress(%s:%d)", host, port);

  mqtt->conn_opts = conn_opts;
  MQTTClient_create(&mqtt->client, address, client_id,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(mqtt->client, mqtt, connlost, msgarrvd, delivered);

  mqtt->notify.data = mqtt;

  int rc = uv_async_init(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->notify,
                         mqtt_notify_to_php);
  if (rc != 0) {
    PLOG_WARN("mqtt(%p) uv_async_init failed with %d", mqtt, rc);
    return -1;
  }

  return 0;
}

static int _free_stopic_item(any_t item, const char *key, any_t data);
static int _free_pwatcher_item(any_t item, const char *key, any_t data);
int proton_mqttclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  ZVAL_PTR_DTOR(&mqtt->options);
  uv_close((uv_handle_t *)&mqtt->notify, NULL);

  if (IS_COROUTINE_WAITFOR(mqtt->pw_connect.wq_work)) {

    // uv_cancel((uv_req_t *)&mqtt->pw_connect.work);
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