/* electron extension for PHP */

#include "common.h"

zend_class_entry *_coroutine_ce;

/* {{{ coroutine proton::go( function $callback )
 */
PHP_FUNCTION(proton_go) {
  zend_fcall_info fci = empty_fcall_info;
  zend_fcall_info_cache fcc = empty_fcall_info_cache;
  zend_resource *r = NULL;
  quark_coroutine_task *task = NULL;
  quark_coroutine_runtime *runtime = quark_get_runtime();
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_FUNC(fci, fcc)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_entry entry = {
      .fci_cache = &fcc,
      .argc = argc,
      .argv = args,
  };

  task = quark_coroutine_create(quark_runtime_main(runtime), &entry, 0, 0);

  zval coroutine;
  object_init_ex(&coroutine, _coroutine_ce);

  proton_object_construct(&coroutine, &task->value);

  ZVAL_COPY(&task->myself, &coroutine);

  quark_coroutine_swap_in(task);

  RETURN_ZVAL(&coroutine, 1, 0);
}
/* }}}*/

/* {{{ coroutine proton::context( )
 */
PHP_FUNCTION(proton_context) {
  quark_coroutine_runtime *runtime = quark_get_runtime();

  quark_coroutine_task *current = quark_coroutine_current();

  ZEND_PARSE_PARAMETERS_NONE();

  if (current == quark_coroutine_get_main()) {
    RETURN_NULL();
  }

  RETURN_ZVAL(&current->myself, 1, 0);
}
/* }}}*/

/* {{{ bool proton_yield()
 */
PHP_FUNCTION(proton_yield) {
  quark_coroutine_runtime *runtime = quark_get_runtime();

  if (quark_coroutine_yield(runtime) != 0) {
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __construct) {
  zend_fcall_info fci = empty_fcall_info;
  zend_fcall_info_cache fcc = empty_fcall_info_cache;
  zend_resource *r = NULL;
  quark_coroutine_task *task = NULL;
  quark_coroutine_runtime *runtime = quark_get_runtime();
  int argc = ZEND_NUM_ARGS() - 1;
  zval *args = NULL;

  ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_FUNC(fci, fcc)
    Z_PARAM_VARIADIC('*', args, argc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_entry entry = {
      .fci_cache = &fcc,
      .argc = argc,
      .argv = args,
  };

  task = quark_coroutine_create(quark_runtime_main(runtime), &entry, 0, 0);

  // insert to wait queue
  // quark_coroutine_ready(runtime, task);

  proton_object_construct(getThis(), &task->value);

  ZVAL_COPY(&task->myself, getThis());
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __destruct) {
  // proton_tcpserver_free(proton_object_get(getThis()));
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, __toString) { RETURN_STRING("{coroutine}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, status) {
  quark_coroutine_task *task =
      (quark_coroutine_task *)proton_object_get(getThis());
  RETURN_LONG(task->status);
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, yield) {
  quark_coroutine_runtime *runtime = quark_get_runtime();

  if (quark_coroutine_yield(runtime) != 0) {
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/** {{{
 */
PHP_METHOD(coroutine, resume) {
  quark_coroutine_runtime *runtime = quark_get_runtime();

  ZEND_PARSE_PARAMETERS_NONE();

  quark_coroutine_task *task =
      (quark_coroutine_task *)proton_object_get(getThis());

  if (task != quark_coroutine_get_main() &&
      quark_coroutine_swap_in(task) == 0) {
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
    PHP_ME(coroutine, yield, NULL, ZEND_ACC_PUBLIC)  // coroutine::yield
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