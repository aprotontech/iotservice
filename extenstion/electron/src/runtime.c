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

/* {{{ void Runtime::start()
 */
PHP_METHOD(runtime, start) {
  proton_coroutine_runtime *runtime = proton_get_runtime();
  RETURN_LONG(proton_runtime_loop(runtime));
}
/* }}} */

/* {{{ void Runtime::stop()
 */
PHP_METHOD(runtime, stop) {
  proton_coroutine_runtime *runtime = proton_get_runtime();
  RETURN_LONG(proton_runtime_stop(runtime));
}
/* }}} */

/* {{{ $exception Runtime::getLastError()
 */
PHP_METHOD(runtime, getLastError) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  RETURN_ZVAL(&runtime->last_error, 0, 0);
}
/* }}} */

/* {{{ $orgHandler Runtime::setErrorHandler(function $handler)
 */
PHP_METHOD(runtime, setErrorHandler) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  zval *func = NULL;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(func)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (!zend_is_callable(func, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "input function must callable");
    return;
  }

  zval orgHandler;
  ZVAL_COPY(&orgHandler, &runtime->error_handler);
  // ZVAL_PTR_DTOR(&runtime->error_handler);
  ZVAL_COPY(&runtime->error_handler, func);

  RETURN_ZVAL(&orgHandler, 0, 0);
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
    PHP_ME(runtime, setErrorHandler, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // runtime::setErrorHandler
    PHP_ME(runtime, getLastError, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // runtime::getLastError
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