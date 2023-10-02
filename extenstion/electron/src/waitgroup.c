/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     waitgroup.c
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-11-06 06:58:00
 *  @version  0
 *  @brief
 *
 * *
 */

#include "common.h"

#include "proton/coroutine/wait_group.h"

PHP_FUNCTION(proton_wait) {
  proton_coroutine_runtime *runtime = proton_get_runtime();
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  for (int i = 0; i < argc; ++i) { // makesure every args is waitable
  }
}

/** {{{
 */
PHP_METHOD(waitgroup, __construct) {

  proton_coroutine_runtime *runtime = proton_get_runtime();
  long mode = WG_WAIT_ONEOF;

  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(mode)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_private_value_t *group = proton_wait_group_create(runtime, mode);

  if (group == NULL) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "alloc new group failed");
    return;
  }

  proton_object_construct(getThis(), group);
}
/* }}} */

/** {{{
 */
PHP_METHOD(waitgroup, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(waitgroup, __toString) { RETURN_STRING("{waitgroup}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(waitgroup, append) {
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;
  proton_private_value_t *group = proton_object_get(getThis());

  ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  for (int i = 0; i < argc; ++i) { // makesure every args is waitable
    if (Z_TYPE(args[i]) != IS_OBJECT || Z_OBJCE(args[i]) != _coroutine_ce) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
                       "input params is not coroutine");
      return;
    }
  }

  int ret = 0;
  for (int i = 0; i < argc; ++i) {
    ret = proton_wait_group_add(group, proton_object_get(&args[i]));
    if (ret != 0) {
      RETURN_LONG(ret);
    }
  }

  RETURN_LONG(0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(waitgroup, wait) {
  proton_wait_group_t *group =
      (proton_wait_group_t *)proton_object_get(getThis());

  ZEND_PARSE_PARAMETERS_NONE();
  if (proton_wait_group_waitfor(&group->value) != 0) { //
    RETURN_NULL();
  }

  if (group->last_notify_task != NULL) {
    proton_notify_item_t *item = group->last_notify_task;
    RETURN_ZVAL(&item->task->myself, 0, 0);
    return;
  }

  RETURN_NULL();
}
/* }}} */

/** {{{
 */
PHP_METHOD(waitgroup, close) {
  ZEND_PARSE_PARAMETERS_NONE();
  RETURN_LONG(proton_wait_group_close(proton_object_get(getThis())));
}
/* }}} */

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in waitgroup_functions[].
 */
const zend_function_entry waitgroup_functions[] = {
    PHP_ME(waitgroup, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // waitgroup::__construct
    PHP_ME(waitgroup, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // waitgroup::__destruct
    PHP_ME(waitgroup, __toString, NULL,
           ZEND_ACC_PUBLIC) // waitgroup::__toString
    PHP_ME(waitgroup, append, NULL,
           ZEND_ACC_PUBLIC) // waitgroup::append
    PHP_ME(waitgroup, wait, NULL,
           ZEND_ACC_PUBLIC) // waitgroup::wait
    PHP_ME(waitgroup, close, NULL,
           ZEND_ACC_PUBLIC) // waitgroup::close
    {NULL, NULL, NULL}      /* Must be the last line in waitgroup_functions[] */
};
/* }}} */

zend_class_entry *_waitgroup_ce;

zend_class_entry *regist_waitgroup_class() {
  zend_class_entry waitgroup;

  INIT_NS_CLASS_ENTRY(waitgroup, PROTON_NAMESPACE, "WaitGroup",
                      waitgroup_functions);
  _waitgroup_ce = zend_register_internal_class_ex(&waitgroup, NULL);

  zend_declare_property_null(_waitgroup_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  zend_declare_class_constant_long(
      _waitgroup_ce, STRING_PARAM_EXPAND("MODE_ALL"), WG_WAIT_ALL);
  zend_declare_class_constant_long(
      _waitgroup_ce, STRING_PARAM_EXPAND("MODE_FIRST"), WG_WAIT_ONEOF);

  return _waitgroup_ce;
}