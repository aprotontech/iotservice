/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     loop.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-18 11:51:29
 *
 */

#include "common.h"

/* {{{ void quark_runtime_loop()
 */
PHP_METHOD(runtime, start) {
  proton_coroutine_runtime *runtime = proton_get_runtime();
  RETURN_LONG(proton_runtime_loop(runtime));
}
/* }}} */

/* {{{ void quark_runtime_stop()
 */
PHP_METHOD(runtime, stop) {
  proton_coroutine_runtime *runtime = proton_get_runtime();
  RETURN_LONG(proton_runtime_stop(runtime));
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_quark_runtime_loop, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_runtime_stop, 0)
ZEND_END_ARG_INFO()

/* {{{ runtime_functions[]
 *
 * Every user visible function must have an entry in runtime_functions[].
 */
const zend_function_entry runtime_functions[] = {
    PHP_ME(runtime, start, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // runtime::start
    PHP_ME(runtime, stop, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // runtime::stop
    {NULL, NULL, NULL} /* Must be the last line in runtime_functions[] */
};
/* }}} */

zend_class_entry *_runtime_ce;
zend_class_entry *regist_runtime_class() {
  zend_class_entry runtime;

  INIT_NS_CLASS_ENTRY(runtime, PROTON_NAMESPACE, "Runtime", runtime_functions);
  _runtime_ce = zend_register_internal_class_ex(&runtime, NULL);

  return _runtime_ce;
}