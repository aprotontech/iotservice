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

  char path[256] = {0};
  char host[1024] = {0};
  struct sockaddr_in addr;
  int len = sizeof(addr);
  uv_tcp_getpeername(&client->tcp, (struct sockaddr *)&addr, &len);
  if (client->current->path.base != NULL) {
    memcpy(path, client->current->path.base,
           min(sizeof(path), client->current->path.len));
  }

  snprintf(host, sizeof(host), "{httprequest.%s(%s:%d%s)}",
           http_method_str(client->current->method), inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port), path);
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

  if (strcasecmp(key, "Path") == 0 && message->path.base != NULL) {
    RETURN_STRINGL(message->path.base, message->path.len);
  } else if (strcasecmp(key, "Method") == 0) {
    RETURN_STRING(http_method_str(message->method));
  } else if (strcasecmp(key, "_POST") == 0) { // get post content
    zval _post;
    ZVAL_NEW_ARR(&_post);
    zend_hash_init(Z_ARRVAL(_post), 5, NULL, ZVAL_PTR_DTOR, 0);

    http_message_get_multipart_content(message, &_post, NULL);

    RETURN_ZVAL(&_post, 0, 0);
  } else if (strcasecmp(key, "_FILES") == 0) { // get post content
    zval _files;
    ZVAL_NEW_ARR(&_files);
    zend_hash_init(Z_ARRVAL(_files), 5, NULL, ZVAL_PTR_DTOR, 0);

    http_message_get_multipart_content(message, NULL, &_files);

    RETURN_ZVAL(&_files, 0, 0);
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
PHP_METHOD(httprequest, getHeaders) {
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
    list_link_t *p = request->message.headers.next;
    while (p != &request->message.headers) {
      proton_header_t *header = container_of(p, proton_header_t, link);
      p = p->next;

      zval value;
      ZVAL_STRINGL(&value, header->value.base, header->value.len);
      zend_string *key = zend_string_init(header->key.base, header->key.len, 0);
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
PHP_METHOD(httprequest, getBody) {
  ZEND_PARSE_PARAMETERS_NONE();

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  if (php_request_wait_parse_finish(request) == 0) {
    RETURN_STR_COPY(http_message_get_raw_body(&request->message));
  }

  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(httprequest, end) {
  long status_code;
  char *body = NULL;
  size_t body_len;
  zval *headers = NULL;

  ZEND_PARSE_PARAMETERS_START(2, 3)
    Z_PARAM_LONG(status_code)
    Z_PARAM_STRING(body, body_len)
    Z_PARAM_OPTIONAL
    Z_PARAM_ARRAY(headers)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  php_http_request_t *request =
      (php_http_request_t *)proton_object_get(getThis());

  int input_headers_count = 0;
  const char **input_headers = get_input_headers(headers, &input_headers_count);

  int rc = proton_httpconnect_write_response(
      &request->connect->value, status_code, input_headers, input_headers_count,
      body, body_len);

  if (input_headers != NULL) {
    free(input_headers);
  }

  RETURN_LONG(rc);
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
           ZEND_ACC_PUBLIC) // httprequest::__get
    PHP_ME(httprequest, getHeaders, NULL,
           ZEND_ACC_PUBLIC) // httprequest::getHeaders
    PHP_ME(httprequest, getBody, NULL,
           ZEND_ACC_PUBLIC)                         // httprequest::getBody
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