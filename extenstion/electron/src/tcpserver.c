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
PHP_METHOD(tcpserver, __construct) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_object_construct(getThis(), proton_tcpserver_create(runtime));
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpserver, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(tcpserver, __toString) { RETURN_STRING("{tcpserver}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(tcpserver, listen) {
  long port;
  char *host = NULL;
  size_t host_len;

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(host, host_len)
    Z_PARAM_LONG(port)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(
      proton_tcpserver_listen(proton_object_get(getThis()), host, port));
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpserver, accept) {
  proton_private_value_t *client = NULL;
  int rc = proton_tcpserver_accept(proton_object_get(getThis()), &client);
  if (rc != 0) {
    PLOG_WARN("tcpserver::accept failed with(%d)", rc);
    RETURN_LONG(rc);
  }

  zval tcpclient;
  object_init_ex(&tcpclient, _tcpclient_ce);

  proton_object_construct(&tcpclient, client);

  RETURN_ZVAL(&tcpclient, 1, 0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(tcpserver, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_tcpserver_close(proton_object_get(getThis())));
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in martin_functions[].
 */
const zend_function_entry tcpserver_functions[] = {
    PHP_ME(tcpserver, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // tcpserver::__construct
    PHP_ME(tcpserver, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // tcpserver::__destruct
    PHP_ME(tcpserver, __toString, NULL,
           ZEND_ACC_PUBLIC)                          // tcpserver::__toString
    PHP_ME(tcpserver, listen, NULL, ZEND_ACC_PUBLIC) // tcpserver::listen
    PHP_ME(tcpserver, accept, NULL, ZEND_ACC_PUBLIC) // tcpserver::accept
    PHP_ME(tcpserver, close, NULL, ZEND_ACC_PUBLIC)  // tcpserver::close
    {NULL, NULL, NULL} /* Must be the last line in tcpserver_functions[] */
};
/* }}} */

zend_class_entry *_tcpserver_ce;
zend_class_entry *regist_tcpserver_class() {
  zend_class_entry tcpserver;

  INIT_NS_CLASS_ENTRY(tcpserver, PROTON_NAMESPACE, "TcpServer",
                      tcpserver_functions);
  _tcpserver_ce = zend_register_internal_class_ex(&tcpserver, NULL);
  // tcpserver_ce->create_object = NULL;

  zend_declare_property_null(_tcpserver_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _tcpserver_ce;
}