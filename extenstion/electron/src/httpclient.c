/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     httpclient.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-22 12:11:43
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(httpclient, __construct) {

  long port = 0;
  char *host = NULL;
  size_t host_len;

  ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_private_value_t *s = proton_httpclient_create(runtime, host, port);
  proton_object_construct(getThis(), s);

  {                   // create connect object
    zval httpconnect; // no need to dec httpconnect.ref_count(httpclient ref it)
    object_init_ex(&httpconnect, _httpconnect_ce);

    proton_object_construct(&httpconnect,
                            &(((proton_http_client_t *)s)->connect->value));
    // Z_TRY_ADDREF(httpconnect); // because first request ref to http connect
  }
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, __toString) {
  proton_http_connect_t *client =
      (proton_http_connect_t *)proton_object_get(getThis());
  char host[40] = {0};
  struct sockaddr_in addr;
  int len = sizeof(addr);
  uv_tcp_getpeername(&client->tcp, (struct sockaddr *)&addr, &len);
  snprintf(host, sizeof(host), "{httpclient(%s:%d)}", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));
  RETURN_STRING(host);
}
/* }}} */

const char **get_input_headers(zval *headers, int *output_headers_count) {
  if (headers != NULL) {
    HashTable *array_hash = HASH_OF(headers);
    int input_headers_count = zend_array_count(array_hash);
    if (input_headers_count != 0) {
      zend_ulong num_idx, valid_idx = 0;
      zend_string *str_idx;
      zval *entry;
      const char **input_headers =
          (const char **)malloc(sizeof(char *) * input_headers_count);
      ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(headers), num_idx, str_idx, entry) {
        if (Z_TYPE_P(entry) != IS_STRING) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
                           "input headers item must be string");
          free(input_headers);
          return NULL;
        }

        zend_string *value = Z_STR_P(entry);
        input_headers[valid_idx++] = value->val;
      }
      ZEND_HASH_FOREACH_END();

      *output_headers_count = valid_idx;

      return input_headers;
    }
  }

  return NULL;
}

/** {{{ http_request httpclient::get($path, $headers = [])
 */
PHP_METHOD(httpclient, get) {
  char *url = NULL;
  size_t url_len;
  zval *headers = NULL;

  ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STRING(url, url_len)
    Z_PARAM_OPTIONAL
    Z_PARAM_ARRAY(headers)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_http_client_t *client =
      (proton_http_client_t *)proton_object_get(getThis());

  int input_headers_count = 0;
  const char **input_headers = get_input_headers(headers, &input_headers_count);

  int rc =
      proton_httpclient_request(&client->value, HTTP_GET, url, input_headers,
                                input_headers_count, NULL, 0);
  if (input_headers != NULL) {
    free(input_headers);
  }

  if (rc == 0) { // create current response
    php_http_request_t *request =
        container_of(client->connect->current, php_http_request_t, message);

    zval httpresponse;
    object_init_ex(&httpresponse, _httpresponse_ce);

    proton_object_construct(&httpresponse, &request->value);

    RETURN_ZVAL(&httpresponse, 0, 0);
  }

  RETURN_NULL();
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, getConnect) {
  ZEND_PARSE_PARAMETERS_NONE();

  proton_http_client_t *client =
      (proton_http_client_t *)proton_object_get(getThis());

  RETURN_ZVAL(&client->connect->value.myself, 1, 0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_proton_httpclient_get, 1)
  ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

/* {{{ httpclient_functions[]
 *
 * Every user visible function must have an entry in httpclient_functions[].
 */
const zend_function_entry httpclient_functions[] = {
    PHP_ME(httpclient, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // httpclient::__construct
    PHP_ME(httpclient, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // httpclient::__destruct
    PHP_ME(httpclient, __toString, NULL,
           ZEND_ACC_PUBLIC) // httpclient::__toString
    PHP_ME(httpclient, getConnect, NULL,
           ZEND_ACC_PUBLIC)                          // httprequest::getConnect
    PHP_ME(httpclient, get, NULL, ZEND_ACC_PUBLIC)   // httpclient::get
    PHP_ME(httpclient, close, NULL, ZEND_ACC_PUBLIC) // httpclient::close
    {NULL, NULL, NULL} /* Must be the last line in httpclient_functions[] */
};
/* }}} */

zend_class_entry *_httpclient_ce;
zend_class_entry *regist_httpclient_class() {
  zend_class_entry httpclient;

  INIT_NS_CLASS_ENTRY(httpclient, PROTON_NAMESPACE, "HttpClient",
                      httpclient_functions);
  _httpclient_ce = zend_register_internal_class_ex(&httpclient, NULL);
  // httpclient_ce->create_object = NULL;

  zend_declare_property_null(_httpclient_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httpclient_ce;
}