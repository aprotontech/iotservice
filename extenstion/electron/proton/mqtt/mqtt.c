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

int mqtt_subscribe(proton_mqtt_client_t *mqtt, const char *topic, int topic_len,
                   int qos) {
  MAKESURE_ON_COROTINUE(mqtt->runtime);
  PLOG_INFO("mqtt(%p) subcribe(%s), qos(%d)\n", mqtt, topic, qos);
  return MQTTClient_subscribe(mqtt->client, topic, qos);
}

int mqtt_publish(proton_mqtt_client_t *mqtt, const char *topic, int topic_len,
                 const char *msg, int msg_len, int qos, int retained, int *dt) {
  MAKESURE_ON_COROTINUE(mqtt->runtime);
  MQTTClient_deliveryToken token;
  int rc = MQTTClient_publish(mqtt->client, topic, msg_len, msg, qos, retained,
                              &token);
  if (rc == MQTTCLIENT_SUCCESS) {
    *dt = (int)token;
  }

  PLOG_INFO("mqtt(%p) publish(%s), msg(%s), ret(%d)\n", mqtt, topic, msg, rc);
  return rc;
}

static void mqtt_connect_task(uv_work_t *req) {
  int rc;
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  if ((rc = MQTTClient_connect(mqtt->client, &mqtt->conn_opts)) !=
      MQTTCLIENT_SUCCESS) {
    PLOG_WARN("mqtt(%p) Failed to connect, return code %d\n", mqtt, rc);
  }

  if (mqtt->connect_work.result != NULL) {
    *((int *)mqtt->connect_work.result) = rc;
  }
}

static void mqtt_connect_result_callback(uv_work_t *req, int status) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;

  proton_coroutine_wakeup(mqtt->runtime, &mqtt->connect_work.wq_work, NULL);
}

static void mqtt_disconnect_task(uv_work_t *req) {

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  int rc = MQTTClient_disconnect(mqtt->client, 5);
  if (rc != MQTTCLIENT_SUCCESS) {
    PLOG_WARN("mqtt(%p) Failed to disconnect, return code %d\n", mqtt, rc);
  }

  if (mqtt->disconnect_work.result != NULL) {
    *((int *)mqtt->disconnect_work.result) = rc;
  }
}

static void mqtt_disconnect_result(uv_work_t *req, int status) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;

  PLOG_DEBUG("mqtt(%p) mqtt_disconnect_result(%d)\n", mqtt, status);
  mqtt->disconnect_work.status = status;
  proton_coroutine_wakeup(mqtt->runtime, &mqtt->disconnect_work.wq_work, NULL);
}

int proton_mqttclient_connect(proton_private_value_t *value, zval *options) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(options);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;

  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (!LL_isspin(&mqtt->connect_work.wq_work.head)) {
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
  mqtt->connect_work.result = &connect_result;

  int rc =
      uv_queue_work(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->connect_work.work,
                    mqtt_connect_task, mqtt_connect_result_callback);
  if (0 != rc) {
    PLOG_WARN("mqtt connect uv_queue_work failed with %d\n", rc);
    return rc;
  }

  if ((rc = proton_coroutine_waitfor(mqtt->runtime, &mqtt->connect_work.wq_work,
                                     NULL)) != 0) {
    return rc;
  }

  if (mqtt->connect_work.status != 0) {
    return -1;
  }

  return connect_result;
}

int proton_mqttclient_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  MAKESURE_ON_COROTINUE(mqtt->runtime);

  if (!LL_isspin(&mqtt->disconnect_work.wq_work.head)) {
    PLOG_WARN("[MQTT] mqtt client is disconnecting...");
    return -1;
  }

  int disconnect_result = -1;
  mqtt->disconnect_work.result = &disconnect_result;

  int rc =
      uv_queue_work(RUNTIME_UV_LOOP(mqtt->runtime), &mqtt->disconnect_work.work,
                    mqtt_disconnect_task, mqtt_disconnect_result);
  if (0 != rc) {
    PLOG_WARN("mqtt disconnect uv_queue_work failed with %d\n", rc);
    return rc;
  }

  if ((rc = proton_coroutine_waitfor(
           mqtt->runtime, &mqtt->disconnect_work.wq_work, NULL)) != 0) {
    return rc;
  }

  if (mqtt->disconnect_work.status != 0) {
    return -1;
  }
  return disconnect_result;
}

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

  mqtt->connect_work.work.data = mqtt;
  mqtt->connect_work.status = 0;
  mqtt->connect_work.result = NULL;
  PROTON_WAIT_OBJECT_INIT(mqtt->connect_work.wq_work);
  mqtt->disconnect_work.work.data = mqtt;
  mqtt->disconnect_work.status = 0;
  mqtt->disconnect_work.result = NULL;
  PROTON_WAIT_OBJECT_INIT(mqtt->disconnect_work.wq_work);

  mqtt->events[PME_DISCONNECT] = NULL;
  mqtt->events[PME_NEW_MESSAGE] = NULL;

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
    PLOG_WARN("mqtt(%p) uv_async_init failed with %d\n", mqtt, rc);
    return -1;
  }

  return 0;
}

int proton_mqttclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)value;
  ZVAL_PTR_DTOR(&mqtt->options);
  uv_close((uv_handle_t *)&mqtt->notify, NULL);

  return 0;
}