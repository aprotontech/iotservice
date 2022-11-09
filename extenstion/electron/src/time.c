/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     time.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-18 11:38:42
 *
 */

#include "common.h"

/* {{{ void proton_sleep( long $time_ms )
 */
PHP_FUNCTION(proton_sleep) {
  long time_ms = 0;
  proton_coroutine_runtime *runtime = proton_get_runtime();

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(time_ms)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  RETURN_LONG(proton_coroutine_sleep(runtime, time_ms));
}
/* }}}*/

/* {{{ coroutinue proton_after( long $time_ms )
 */
PHP_FUNCTION(proton_after) {
  long time_ms = 0;
  proton_coroutine_runtime *runtime = proton_get_runtime();

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(time_ms)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  char funcname[100] = {0};
  snprintf(funcname, sizeof(funcname), "%s\\sleep", PROTON_NAMESPACE);

  zval func, params[1];
  ZVAL_STR(&func, zend_string_init(funcname, strlen(funcname), 0));
  ZVAL_LONG(&params[0], time_ms);

  proton_coroutine_entry entry = {
      .argc = 1,
      .argv = params,
  };

  ZVAL_COPY(&entry.func, &func);

  proton_coroutine_task *task = proton_coroutine_create(runtime, &entry, 0, 0);

  if (task == NULL) {
    RETURN_NULL();
  }

  zval coroutine;
  object_init_ex(&coroutine, _coroutine_ce);

  proton_object_construct(&coroutine, &task->value);

  proton_coroutine_resume(runtime, task);

  // [coroutine] is holding refcount, so don't add new ref
  RETURN_ZVAL(&coroutine, 0, 0);
}
/* }}}*/