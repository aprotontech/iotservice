/* electron extension for PHP */

#include "common.h"

zend_class_entry *_coroutine_ce;

/* {{{ coroutine proton::go( function $callback )
 */
PHP_FUNCTION(proton_go) {
  zval *func;
  zend_resource *r = NULL;
  proton_coroutine_task *task = NULL;
  proton_coroutine_runtime *runtime = proton_get_runtime();
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_ZVAL(func)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (!zend_is_callable(func, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                     "input function must be callable");
    return;
  }

  proton_coroutine_entry entry = {
      .argc = argc,
      .argv = args,
  };

  ZVAL_COPY(&entry.func, func);

  task = proton_coroutine_create(runtime, &entry, 0, 0);

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

/* {{{ coroutine proton::context( )
 */
PHP_FUNCTION(proton_context) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(runtime);

  ZEND_PARSE_PARAMETERS_NONE();

  PLOG_DEBUG("get context(%p)", current);
  if (!IS_REAL_COROUTINE(current)) {
    PLOG_WARN("current task is main coroutine");
    RETURN_NULL();
  }

  RETURN_ZVAL(&current->value.myself, 1, 0);
}
/* }}}*/

/* {{{ bool proton_yield()
 */
PHP_FUNCTION(proton_yield) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  if (proton_coroutine_yield(runtime) != 0) {
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __construct) {
  zval *func;
  zend_resource *r = NULL;
  proton_coroutine_task *task = NULL;
  proton_coroutine_runtime *runtime = proton_get_runtime();
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;

  PLOG_DEBUG("__construct");

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_ZVAL(func)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (!zend_is_callable(func, IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "input function must callable");
    return;
  }

  proton_coroutine_entry entry = {
      .argc = argc,
      .argv = args,
  };

  ZVAL_COPY(&entry.func, func);

  task = proton_coroutine_create(runtime, &entry, 0, 0);
  if (task == NULL) {
    zend_throw_exception(NULL, "create coroutine task failed", 0);
    return;
  }

  proton_object_construct(getThis(), &task->value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __destruct) { proton_object_destruct(getThis()); }
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __toString) { RETURN_STRING("{coroutine}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, status) {
  proton_coroutine_task *task =
      (proton_coroutine_task *)proton_object_get(getThis());
  RETURN_LONG(task->status);
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, pause) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  if (proton_coroutine_yield(runtime) != 0) {
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, resume) {
  proton_coroutine_runtime *runtime = proton_get_runtime();

  ZEND_PARSE_PARAMETERS_NONE();

  proton_coroutine_task *task =
      (proton_coroutine_task *)proton_object_get(getThis());

  if (proton_coroutine_resume(runtime, task) == 0) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, get) { ZEND_PARSE_PARAMETERS_NONE(); }
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, set) { ZEND_PARSE_PARAMETERS_NONE(); }
/* }}} */

/* {{{ coroutine_functions[]
 *
 * Every user visible function must have an entry in tcpclient_functions[].
 */
const zend_function_entry coroutine_functions[] = {
    PHP_ME(coroutine, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // coroutine::__construct
    PHP_ME(coroutine, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // coroutine::__destruct
    PHP_ME(coroutine, __toString, NULL,
           ZEND_ACC_PUBLIC)                          // coroutine::__toString
    PHP_ME(coroutine, get, NULL, ZEND_ACC_PUBLIC)    // coroutine::get
    PHP_ME(coroutine, set, NULL, ZEND_ACC_PUBLIC)    // coroutine::set
    PHP_ME(coroutine, pause, NULL, ZEND_ACC_PUBLIC)  // coroutine::pause
    PHP_ME(coroutine, resume, NULL, ZEND_ACC_PUBLIC) // coroutine::resume
    PHP_ME(coroutine, status, NULL, ZEND_ACC_PUBLIC) // coroutine::status
    {NULL, NULL, NULL} /* Must be the last line in coroutine_functions[] */
};
/* }}} */

zend_class_entry *regist_coroutine_class() {
  zend_class_entry coroutine;

  INIT_NS_CLASS_ENTRY(coroutine, PROTON_NAMESPACE, "coroutine",
                      coroutine_functions);
  _coroutine_ce = zend_register_internal_class_ex(&coroutine, NULL);
  // tcpserver_ce->create_object = NULL;

  zend_declare_property_null(_coroutine_ce,
                             ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  return _coroutine_ce;
}