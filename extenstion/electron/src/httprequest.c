/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     httprequest.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-04 04:36:08
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(httprequest, __construct) {}
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, __toString) {
  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  proton_http_connect_t *client = request->connect;

  char host[1024] = {0};
  struct sockaddr_in addr;
  int len = sizeof(addr);
  uv_tcp_getpeername(&client->tcp, (struct sockaddr *)&addr, &len);
  snprintf(host, sizeof(host), "{httprequest.%s(%s:%d%s)}",
           http_method_str(client->current->method), inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port), client->current->path);
  RETURN_STRING(host);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, __get) {
  char *key = NULL;
  size_t key_len;

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  proton_http_message_t *message = &request->message;
  if (message == NULL) {
    RETURN_NULL();
  }

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(key, key_len)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (strcasecmp(key, "Path") == 0 && message->path != NULL) {
    RETURN_STRING(message->path);
  }

  RETURN_NULL();
}
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, getConnect) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  RETURN_ZVAL(&request->connect->value.myself, 1, 0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, end) {
  long status_code;
  char *body = NULL;
  size_t body_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(status_code)
    Z_PARAM_STRING(body, body_len)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  RETURN_LONG(proton_httpconnect_write_response(
      &request->connect->value, status_code, NULL, 0, body, body_len));
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_proton_httprequest_get, 1)
  ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

/* {{{ httprequest_functions[]
 *
 * Every user visible function must have an entry in httprequest_functions[].
 */
const zend_function_entry httprequest_functions[] = {
    PHP_ME(httprequest, __construct, NULL,
           ZEND_ACC_PROTECTED | ZEND_ACC_CTOR) // httprequest::__construct
    PHP_ME(httprequest, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // httprequest::__destruct
    PHP_ME(httprequest, getConnect, NULL,
           ZEND_ACC_PUBLIC) // httprequest::getConnect
    PHP_ME(httprequest, __toString, NULL,
           ZEND_ACC_PUBLIC) // httprequest::__toString
    PHP_ME(httprequest, __get, arginfo_proton_httprequest_get,
           ZEND_ACC_PUBLIC)                         // httprequest::__get
    PHP_ME(httprequest, end, NULL, ZEND_ACC_PUBLIC) // httprequest::end
    {NULL, NULL, NULL} /* Must be the last line in httprequest_functions[] */
};
/* }}} */

zend_class_entry *_httprequest_ce;
zend_class_entry *regist_httprequest_class() {
  zend_class_entry httprequest;

  INIT_NS_CLASS_ENTRY(httprequest, PROTON_NAMESPACE, "HttpRequest",
                      httprequest_functions);
  _httprequest_ce = zend_register_internal_class_ex(&httprequest, NULL);

  zend_declare_property_null(_httprequest_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httprequest_ce;
}