/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     mqttclient.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-15 10:22:58
 *
 */

#include "common.h"
#include "proton/mqtt/mqtt_client.h"

int on_mqttclient_recv_publish_message(proton_mqtt_client_t *mqtt,
                                       zval *message, zval *callback) {

  proton_private_value_t *obj = &mqtt->value;
  if (Z_TYPE_P(&obj->myself) != IS_OBJECT) {
    PLOG_INFO("mqtt client (%p) is not object", mqtt);
    return -1;
  }

  zval rv;
  zval *self = &obj->myself;

  if (callback != NULL &&
      zend_is_callable(callback, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    zval params[2];

    ZVAL_COPY(&params[0], self);
    ZVAL_COPY(&params[1], message);

    proton_coroutine_entry entry = {
        .argc = 2,
        .argv = params,
    };
    ZVAL_COPY(&entry.func, callback);

    proton_coroutine_runtime *runtime = proton_get_runtime();

    proton_coroutine_task *task =
        proton_coroutine_create(runtime, &entry, 0, 0);

    zval coroutine;
    object_init_ex(&coroutine, _coroutine_ce);

    proton_object_construct(&coroutine, &task->value);

    Z_TRY_DELREF(params[0]);
    Z_TRY_DELREF(params[1]);

    proton_coroutine_resume(NULL, task);

    PLOG_INFO("switch to callback done try dtor coroutine(%lu)", task->cid);
    ZVAL_PTR_DTOR(&coroutine); // release it

    /*
        zval status, code, message;
        ZVAL_NEW_ARR(&status);
        zend_hash_init(Z_ARRVAL(status), 2, NULL, ZVAL_PTR_DTOR, 0);

        ZVAL_LONG(&code, 0);
        zend_hash_add(Z_ARRVAL(status),
                      zend_string_init("status", sizeof("status") - 1, 0),
       &code);

        ZVAL_STRINGL(&message, "closed", sizeof("closed") - 1);
        zend_hash_add(Z_ARRVAL(status),
                      zend_string_init("message", sizeof("message") - 1, 0),
                      &message);

        if (proton_channel_try_push(mqtt->status_channel, &status) != 0) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
                           "push connect status message to channel failed");
        }
        Z_TRY_DELREF(status);*/
  }

  return 0;
}

/** {{{
 */
PHP_METHOD(mqttclient, __construct) {

  long port;
  char *host = NULL;
  size_t host_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_mqtt_client_t *client =
      (proton_mqtt_client_t *)qmalloc(sizeof(proton_mqtt_client_t));
  if (proton_mqttclient_init(runtime, client, host, port) != 0) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "init mqtt client failed");
    qfree(client);
    return;
  }

  proton_object_construct(getThis(), &client->value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, __toString) { RETURN_STRING("{mqttclient}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, connect) {
  zval *options = NULL;
  zval *handler = NULL;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY(options)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_mqttclient_connect_options_t conn_options;

  if (parse_mqttclient_options(options, &conn_options) != 0) {
    RETURN_LONG(-1);
  }

  int ret =
      proton_mqttclient_connect(proton_object_get(getThis()), &conn_options);

  RETURN_LONG(ret);
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, publish) {
  char *topic = NULL, *message = NULL;
  size_t topic_len, message_len;
  long qos = 0, retained = 0;

  ZEND_PARSE_PARAMETERS_START(2, 4)
    Z_PARAM_STRING(topic, topic_len)
    Z_PARAM_STRING(message, message_len)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(qos)
    Z_PARAM_LONG(retained)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(proton_mqttclient_publish(proton_object_get(getThis()), topic,
                                        topic_len, message, message_len, qos,
                                        retained));
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, subscribe) {
  char *topic = NULL;
  size_t topic_len;
  long qos = 0;
  zval *callback = NULL;

  ZEND_PARSE_PARAMETERS_START(2, 3)
    Z_PARAM_STRING(topic, topic_len)
    Z_PARAM_ZVAL(callback)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(qos)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (callback == NULL ||
      !zend_is_callable(callback, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "input handle must callable");
  }

  RETURN_LONG(proton_mqttclient_subscribe(proton_object_get(getThis()), topic,
                                          topic_len, qos, callback));
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_mqttclient_close(proton_object_get(getThis())));
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, isConnected) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_BOOL(proton_mqttclient_get_status(proton_object_get(getThis())) ==
              MQTT_CLIENT_CONNECTED);
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, loop) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_mqttclient_loop(proton_object_get(getThis()),
                                     on_mqttclient_recv_publish_message));
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in mqttclient_functions[].
 */
const zend_function_entry mqttclient_functions[] = {
    PHP_ME(mqttclient, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // mqttclient::__construct
    PHP_ME(mqttclient, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // mqttclient::__destruct
    PHP_ME(mqttclient, __toString, NULL,
           ZEND_ACC_PUBLIC) // mqttclient::__toString
    PHP_ME(mqttclient, isConnected, NULL,
           ZEND_ACC_PUBLIC)                         // mqttclient::isConnected
    PHP_ME(mqttclient, loop, NULL, ZEND_ACC_PUBLIC) // mqttclient::loop
    PHP_ME(mqttclient, connect, NULL, ZEND_ACC_PUBLIC) // mqttclient::connect
    PHP_ME(mqttclient, publish, NULL, ZEND_ACC_PUBLIC) // mqttclient::publish
    PHP_ME(mqttclient, subscribe, NULL,
           ZEND_ACC_PUBLIC)                          // mqttclient::subscribe
    PHP_ME(mqttclient, close, NULL, ZEND_ACC_PUBLIC) // mqttclient::close
    {NULL, NULL, NULL} /* Must be the last line in mqttclient_functions[] */
};
/* }}} */

zend_class_entry *_mqttclient_ce;

zend_class_entry *regist_mqttclient_class() {
  zend_class_entry mqttclient;

  INIT_NS_CLASS_ENTRY(mqttclient, PROTON_NAMESPACE, "MqttClient",
                      mqttclient_functions);
  _mqttclient_ce = zend_register_internal_class_ex(&mqttclient, NULL);

  zend_declare_property_null(_mqttclient_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _mqttclient_ce;
}