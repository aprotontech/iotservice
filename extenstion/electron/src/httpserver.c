/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     httpserver.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-20 11:05:17
 *
 */

#include "common.h"

void php_new_httpserver_client(proton_private_value_t *server,
                               proton_private_value_t *client) {
  QUARK_DEBUG_PRINT("new client");

  zval httpclient;
  object_init_ex(&httpclient, _httpclient_ce);

  proton_object_construct(&httpclient, client);

  zval rv;
  zval *self = &((proton_http_server_t *)server)->myself;
  zval *callback = zend_read_property(
      Z_OBJCE_P(self), self, ZEND_STRL(PROTON_HTTPSERVER_DEFAULT_ROUTER_VALUE),
      0 TSRMLS_CC, &rv);

  if (callback != NULL) {
    zval retval;
    zval params[2];

    ZVAL_NULL(&retval);
    ZVAL_COPY(&params[1], self);
    ZVAL_COPY(&params[1], &httpclient);

    if (call_user_function_ex(EG(function_table), NULL, callback, &retval, 2,
                              params, 0, NULL TSRMLS_CC) != SUCCESS) {
      QUARK_LOGGER("callback new httpclient failed");
    }

    Z_TRY_DELREF(params[0]);
    Z_TRY_DELREF(params[1]);
    zval_ptr_dtor(&retval);
  }
}

/** {{{
 */
PHP_METHOD(httpserver, __construct) {
  long port;
  char *host = NULL;
  size_t host_len;
  zval *handler = NULL;

  ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
    Z_PARAM_ZVAL(handler)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (!zend_is_callable(handler, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "input handle must callable");
    return;
  }

  quark_coroutine_runtime *runtime = quark_get_runtime();

  proton_http_server_config_t config = {
      .handler = php_new_httpserver_client, .host = host, .port = port};

  proton_private_value_t *s = proton_httpserver_create(runtime, &config);
  proton_object_construct(getThis(), s);

  zend_update_property(Z_OBJCE_P(getThis()), getThis(),
                       ZEND_STRL(PROTON_HTTPSERVER_DEFAULT_ROUTER_VALUE),
                       handler TSRMLS_CC);

  ZVAL_COPY(&((proton_http_server_t *)s)->myself, getThis());
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpserver, __destruct) {
  proton_httpserver_free(proton_object_get(getThis()));
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpserver, __toString) { RETURN_STRING("{httpserver}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(httpserver, start) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_httpserver_start(proton_object_get(getThis())));
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpserver, stop) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_httpserver_stop(proton_object_get(getThis())));
}
/* }}} */

/* {{{ httpserver_functions[]
 *
 * Every user visible function must have an entry in httpserver_functions[].
 */
const zend_function_entry httpserver_functions[] = {
    PHP_ME(httpserver, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // httpserver::__construct
    PHP_ME(httpserver, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // httpserver::__destruct
    PHP_ME(httpserver, __toString, NULL,
           ZEND_ACC_PUBLIC)                          // httpserver::__toString
    PHP_ME(httpserver, start, NULL, ZEND_ACC_PUBLIC) // httpserver::start
    PHP_ME(httpserver, stop, NULL, ZEND_ACC_PUBLIC)  // httpserver::stop
    {NULL, NULL, NULL} /* Must be the last line in httpserver_functions[] */
};
/* }}} */

zend_class_entry *_httpserver_ce;
zend_class_entry *regist_httpserver_class() {
  zend_class_entry httpserver;

  INIT_NS_CLASS_ENTRY(httpserver, PROTON_NAMESPACE, "httpserver",
                      httpserver_functions);
  _httpserver_ce = zend_register_internal_class_ex(&httpserver, NULL);
  // httpserver_ce->create_object = NULL;

  zend_declare_property_null(_httpserver_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  zend_declare_property_null(_httpserver_ce,
                             ZEND_STRL(PROTON_HTTPSERVER_DEFAULT_ROUTER_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httpserver_ce;
}