/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     fsevent.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-03-12 09:16:01
 *
 */

#include "common.h"

/** {{{
 */
PHP_METHOD(fsevent, __construct) {
  PLOG_DEBUG("construct");
  proton_coroutine_runtime *runtime = proton_get_runtime();
}
/* }}} */

/** {{{
 */
PHP_METHOD(fsevent, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(fsevent, __toString) { RETURN_STRING("{fsevent}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(fsevent, watch) { RETURN_LONG(0); }
/* }}} */

/** {{{
 */
PHP_METHOD(fsevent, stop) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(0);
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in fsevent_functions[].
 */
const zend_function_entry fsevent_functions[] = {
    PHP_ME(fsevent, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // fsevent::__construct
    PHP_ME(fsevent, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // fsevent::__destruct
    PHP_ME(fsevent, __toString, NULL,
           ZEND_ACC_PUBLIC)                       // fsevent::__toString
    PHP_ME(fsevent, watch, NULL, ZEND_ACC_PUBLIC) // fsevent::watch
    PHP_ME(fsevent, stop, NULL, ZEND_ACC_PUBLIC)  // fsevent::stop
    {NULL, NULL, NULL} /* Must be the last line in fsevent_functions[] */
};
/* }}} */

zend_class_entry *_fsevent_ce;

zend_class_entry *regist_fsevent_class() {
  zend_class_entry fsevent;

  INIT_NS_CLASS_ENTRY(fsevent, PROTON_NAMESPACE, "FileSystemEvent",
                      fsevent_functions);
  _fsevent_ce = zend_register_internal_class_ex(&fsevent, NULL);

  zend_declare_property_null(_fsevent_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _fsevent_ce;
}