/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     httpconnect.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-06 10:07:10
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(httpconnect, __construct) {}
/* }}} */

/** {{{
 */
PHP_METHOD(httpconnect, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(httpconnect, __toString) {
  proton_http_connect_t *connect =
      (proton_http_connect_t *)proton_object_get(getThis());
  char host[40] = {0};
  struct sockaddr_in addr;
  int len = sizeof(addr);
  uv_tcp_getpeername(&connect->tcp, (struct sockaddr *)&addr, &len);
  snprintf(host, sizeof(host), "{httpconnect(%s:%d)}", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));
  RETURN_STRING(host);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpconnect, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

/* {{{ httpconnect_functions[]
 *
 * Every user visible function must have an entry in httpconnect_functions[].
 */
const zend_function_entry httpconnect_functions[] = {
    PHP_ME(httpconnect, __construct, NULL,
           ZEND_ACC_PROTECTED | ZEND_ACC_CTOR) // httpconnect::__construct
    PHP_ME(httpconnect, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // httpconnect::__destruct
    PHP_ME(httpconnect, __toString, NULL,
           ZEND_ACC_PUBLIC)                           // httpconnect::__toString
    PHP_ME(httpconnect, close, NULL, ZEND_ACC_PUBLIC) // httpconnect::close
    {NULL, NULL, NULL} /* Must be the last line in httpconnect_functions[] */
};
/* }}} */

zend_class_entry *_httpconnect_ce;
zend_class_entry *regist_httpconnect_class() {
  zend_class_entry httpconnect;

  INIT_NS_CLASS_ENTRY(httpconnect, PROTON_NAMESPACE, "httpconnect",
                      httpconnect_functions);
  _httpconnect_ce = zend_register_internal_class_ex(&httpconnect, NULL);

  zend_declare_property_null(_httpconnect_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httpconnect_ce;
}