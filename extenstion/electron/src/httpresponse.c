/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     httpresponse.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-11 11:44:59
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(httpresponse, __construct) {}
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, __toString) {
  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  proton_http_connect_t *client = request->connect;

  char host[1024] = {0};
  struct sockaddr_in addr;
  int len = sizeof(addr);
  uv_tcp_getpeername(&client->tcp, (struct sockaddr *)&addr, &len);
  snprintf(host, sizeof(host), "{httpresponse.%s(%s:%d%s)}",
           http_method_str(client->current->method), inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port), client->current->path);
  RETURN_STRING(host);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, __get) {
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

  if (strcasecmp(key, "StatusCode") == 0) {
    RETURN_LONG(message->parser.status_code);
  }

  RETURN_NULL();
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, getBody) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  if (php_request_wait_parse_finish(request) == 0) {
    // get all bodys
    RETURN_STR(proton_link_to_string(&request->message.response_body));
  }

  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, getHeaders) {
  zend_bool skip_same = 1;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(skip_same)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  if (php_request_wait_parse_finish(request) == 0) {
    zval headers;
    ZVAL_NEW_ARR(&headers);
    zend_hash_init(Z_ARRVAL(headers), 10, NULL, ZVAL_PTR_DTOR, 0);
    list_link_t *p = request->message.response_headers.next;
    while (p != &request->message.response_headers) {
      proton_header_t *header = container_of(p, proton_header_t, link);
      p = p->next;

      zval value;
      ZVAL_STR(&value,
               zend_string_init(header->value, strlen(header->value), 0));
      zend_string *key = zend_string_init(header->key, strlen(header->key), 0);
      if (skip_same) {
        zend_hash_add(Z_ARRVAL(headers), key, &value);
      } else {
        zend_hash_add_new(Z_ARRVAL(headers), key, &value);
      }
    }

    RETURN_ZVAL(&headers, 0, 0);
  }

  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpresponse, getConnect) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  RETURN_ZVAL(&(request->connect->value.myself), 1, 0);
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_proton_httpresponse_get, 1)
  ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

/* {{{ httpresponse_functions[]
 *
 * Every user visible function must have an entry in httpresponse_functions[].
 */
const zend_function_entry httpresponse_functions[] = {
    PHP_ME(httpresponse, __construct, NULL,
           ZEND_ACC_PROTECTED | ZEND_ACC_CTOR) // httpresponse::__construct
    PHP_ME(httpresponse, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // httpresponse::__destruct
    PHP_ME(httpresponse, getConnect, NULL,
           ZEND_ACC_PUBLIC) // httpresponse::getConnect
    PHP_ME(httpresponse, __toString, NULL,
           ZEND_ACC_PUBLIC) // httpresponse::__toString
    PHP_ME(httpresponse, getHeaders, NULL,
           ZEND_ACC_PUBLIC) // httpresponse::getHeaders
    PHP_ME(httpresponse, getBody, NULL,
           ZEND_ACC_PUBLIC) // httpresponse::getBody
    PHP_ME(httpresponse, __get, arginfo_proton_httpresponse_get,
           ZEND_ACC_PUBLIC) // httpresponse::__get
    {NULL, NULL, NULL} /* Must be the last line in httpresponse_functions[] */
};
/* }}} */

zend_class_entry *_httpresponse_ce;
zend_class_entry *regist_httpresponse_class() {
  zend_class_entry httpresponse;

  INIT_NS_CLASS_ENTRY(httpresponse, PROTON_NAMESPACE, "HttpResponse",
                      httpresponse_functions);
  _httpresponse_ce = zend_register_internal_class_ex(&httpresponse, NULL);

  zend_declare_property_null(_httpresponse_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httpresponse_ce;
}