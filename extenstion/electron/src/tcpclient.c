/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     tcpserver.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-16 08:56:01
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(tcpclient, __construct) {
  PLOG_DEBUG("construct");
  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_object_construct(getThis(), proton_tcpclient_create(runtime));
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, __destruct) {
  proton_private_value_t *value = proton_object_get(getThis());

  proton_object_destruct(getThis());

  proton_tcpclient_free(value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, __toString) { RETURN_STRING("{tcpclient}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, connect) {
  long port;
  char *host = NULL;
  size_t host_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(
      proton_tcpclient_connect(proton_object_get(getThis()), host, port));
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, write) {
  char *content = NULL;
  size_t content_len;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(content, content_len)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG((long)proton_tcpclient_write(proton_object_get(getThis()),
                                           content, content_len));
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, read) {
  long len;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(len)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  zend_string *buffer = zend_string_alloc(len, 0);

  int rc = proton_tcpclient_read(proton_object_get(getThis()), buffer->val,
                                 buffer->len);
  if (rc >= 0) {
    zval val;
    buffer->len = rc;
    ZVAL_NEW_STR(&val, buffer);
    RETURN_ZVAL(&val, 1, 0);
  }

  zend_string_release(buffer);
  RETURN_LONG(rc);
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpclient, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_tcpclient_close(proton_object_get(getThis())));
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in tcpclient_functions[].
 */
const zend_function_entry tcpclient_functions[] = {
    PHP_ME(tcpclient, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // tcpclient::__construct
    PHP_ME(tcpclient, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // tcpclient::__destruct
    PHP_ME(tcpclient, __toString, NULL,
           ZEND_ACC_PUBLIC)                           // tcpclient::__toString
    PHP_ME(tcpclient, connect, NULL, ZEND_ACC_PUBLIC) // tcpclient::connect
    PHP_ME(tcpclient, write, NULL, ZEND_ACC_PUBLIC)   // tcpclient::write
    PHP_ME(tcpclient, read, NULL, ZEND_ACC_PUBLIC)    // tcpclient::read
    PHP_ME(tcpclient, close, NULL, ZEND_ACC_PUBLIC)   // tcpclient::close
    {NULL, NULL, NULL} /* Must be the last line in tcpclient_functions[] */
};
/* }}} */

zend_class_entry *_tcpclient_ce;

zend_class_entry *regist_tcpclient_class() {
  zend_class_entry tcpclient;

  INIT_NS_CLASS_ENTRY(tcpclient, PROTON_NAMESPACE, "tcpclient",
                      tcpclient_functions);
  _tcpclient_ce = zend_register_internal_class_ex(&tcpclient, NULL);

  zend_declare_property_null(_tcpclient_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _tcpclient_ce;
}