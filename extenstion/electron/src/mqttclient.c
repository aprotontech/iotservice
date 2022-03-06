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
#include "proton/mqtt/mqtt.h"

/** {{{
 */
PHP_METHOD(mqttclient, __construct) {

  long port;
  char *host = NULL, *client_id = NULL;
  size_t host_len, client_id_len;

  ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_STRING(client_id, client_id_len)
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_mqtt_client_t *client =
      (proton_mqtt_client_t *)qmalloc(sizeof(proton_mqtt_client_t));
  if (proton_mqttclient_init(runtime, client, client_id, host, port) != 0) {
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
  zval *channel = NULL;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_ARRAY(options)
    Z_PARAM_OPTIONAL
    Z_PARAM_OBJECT_OF_CLASS(channel, _channel_ce)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(proton_mqttclient_connect(proton_object_get(getThis()), options,
                                        proton_object_get(channel)));
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

  int dt = 0;
  int rc =
      proton_mqttclient_publish(proton_object_get(getThis()), topic, topic_len,
                                message, message_len, qos, retained, &dt);

  RETURN_LONG(rc);
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, subscribe) {
  char *topic = NULL;
  size_t topic_len;
  long qos = 0;
  zval *channel = NULL;

  ZEND_PARSE_PARAMETERS_START(2, 3)
    Z_PARAM_STRING(topic, topic_len)
    Z_PARAM_OBJECT_OF_CLASS(channel, _channel_ce)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(qos)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(proton_mqttclient_subscribe(proton_object_get(getThis()), topic,
                                          topic_len, qos,
                                          proton_object_get(channel)));
}
/* }}} */

/** {{{
 */
PHP_METHOD(mqttclient, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_mqttclient_close(proton_object_get(getThis())));
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
           ZEND_ACC_PUBLIC)                            // mqttclient::__toString
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