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
  quark_coroutine_runtime *runtime = quark_get_runtime();

  proton_object_construct(getThis(), proton_httpclient_create(runtime));

  // ZVAL_COPY(&((proton_http_client_t *)s)->myself, getThis());
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, __destruct) {
  QUARK_DEBUG_PRINT("__destruct");
  proton_httpclient_free(proton_object_get(getThis()));
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, __toString) { RETURN_STRING("{httpclient}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, get) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, end) {
  long status_code;
  char *body = NULL;
  size_t body_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(status_code)
    Z_PARAM_STRING(body, body_len)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_runtime *runtime = quark_get_runtime();
  RETURN_LONG(proton_httpclient_write_response(
      proton_object_get(getThis()), status_code, NULL, 0, body, body_len));
}
/* }}} */

/** {{{
 */
PHP_METHOD(httpclient, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

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
           ZEND_ACC_PUBLIC)                          // httpclient::__toString
    PHP_ME(httpclient, get, NULL, ZEND_ACC_PUBLIC)   // httpclient::get
    PHP_ME(httpclient, end, NULL, ZEND_ACC_PUBLIC)   // httpclient::end
    PHP_ME(httpclient, close, NULL, ZEND_ACC_PUBLIC) // httpclient::close
    {NULL, NULL, NULL} /* Must be the last line in httpclient_functions[] */
};
/* }}} */

zend_class_entry *_httpclient_ce;
zend_class_entry *regist_httpclient_class() {
  zend_class_entry httpclient;

  INIT_NS_CLASS_ENTRY(httpclient, PROTON_NAMESPACE, "httpclient",
                      httpclient_functions);
  _httpclient_ce = zend_register_internal_class_ex(&httpclient, NULL);
  // httpclient_ce->create_object = NULL;

  zend_declare_property_null(_httpclient_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _httpclient_ce;
}