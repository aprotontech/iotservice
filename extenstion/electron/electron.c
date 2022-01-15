/* electron extension for PHP */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_electron.h"
#include "ext/standard/info.h"
#include "proton/coroutine/task.h"
#include "proton/libuv/runtime.h"
#include "proton/include/electron.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()                                           \
  ZEND_PARSE_PARAMETERS_START(0, 0)                                            \
  ZEND_PARSE_PARAMETERS_END()
#endif

static int pc_resource_handle;

void static destruct_proton_coroutine(zend_resource *rsrc) {
  QUARK_LOGGER("[ELECTRON] coroutinue destruct");
  quark_coroutine_task *task = (quark_coroutine_task *)rsrc->ptr;

  quark_coroutine_destory(task);
}

/* {{{ void quark_coroutine_yield(resource $qc)
 */
PHP_FUNCTION(quark_coroutine_yield) {
  quark_coroutine_runtime *runtime = quark_get_runtime();

  if (quark_coroutine_yield(runtime) != 0) {
    RETURN_FALSE;
  }

  RETURN_TRUE;
}
/* }}} */

/* {{{ void quark_runtime_loop()
 */
PHP_FUNCTION(quark_runtime_loop) {
  quark_coroutine_runtime *runtime = quark_get_runtime();
  RETURN_LONG(quark_runtime_loop(runtime));
}
/* }}} */

/* {{{ void quark_runtime_stop()
 */
PHP_FUNCTION(quark_runtime_stop) {
  quark_coroutine_runtime *runtime = quark_get_runtime();
  RETURN_LONG(quark_runtime_stop(runtime));
}
/* }}} */

/* {{{ resource quark_coroutine_sleep( long $time_ms )
 */
PHP_FUNCTION(quark_coroutine_sleep) {
  long time_ms = 0;
  quark_coroutine_runtime *runtime = quark_get_runtime();

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(time_ms)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_sleep(runtime, time_ms);

  RETURN_TRUE;
}
/* }}}*/

/* {{{ resource quark_coroutine_create( function $callback )
 */
PHP_FUNCTION(quark_coroutine_create) {

  zend_fcall_info fci = empty_fcall_info;
  zend_fcall_info_cache fcc = empty_fcall_info_cache;
  zend_resource *r = NULL;
  quark_coroutine_task *task = NULL;
  quark_coroutine_runtime *runtime = quark_get_runtime();

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_FUNC(fci, fcc)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  quark_coroutine_entry entry = {
      .fci_cache = &fcc,
      .argc = 0,
      .argv = NULL,
  };

  task = quark_coroutine_create(quark_runtime_main(runtime), &entry, 0, 0);

  r = zend_register_resource(task, pc_resource_handle);

  ZVAL_RES(&task->myself, r);

  quark_coroutine_swap_in(task);

  RETURN_RES(r);
}
/* }}}*/

PHP_MINIT_FUNCTION(electron) {
  pc_resource_handle = zend_register_list_destructors_ex(
      destruct_proton_coroutine, NULL, PHP_COROUTINE_RESOURCE_NAME,
      module_number);
  if (pc_resource_handle == FAILURE) {
    QUARK_LOGGER("regist resource %s failed\n", PHP_COROUTINE_RESOURCE_NAME);
  }
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(electron) {
#if defined(ZTS) && defined(COMPILE_DL_ELECTRON)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(electron) {
  php_info_print_table_start();
  php_info_print_table_header(2, "electron support", "enabled");
  php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_yield, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_runtime_loop, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_runtime_stop, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_sleep, 1)
  ZEND_ARG_INFO(0, time_ms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_create, 1)
  ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ electron_functions[]
 */
static const zend_function_entry electron_functions[] = {
    PHP_FE(quark_runtime_loop, arginfo_quark_runtime_loop)         // loop
    PHP_FE(quark_runtime_stop, arginfo_quark_runtime_stop)         // stop
    PHP_FE(quark_coroutine_sleep, arginfo_quark_coroutine_sleep)   // sleep
    PHP_FE(quark_coroutine_yield, arginfo_quark_coroutine_yield)   // yield
    PHP_FE(quark_coroutine_create, arginfo_quark_coroutine_create) // create
    PHP_FE_END                                                     // eof
};
/* }}} */

/* {{{ electron_module_entry
 */
zend_module_entry electron_module_entry = {
    STANDARD_MODULE_HEADER,
    "electron",           /* Extension name */
    electron_functions,   /* zend_function_entry */
    PHP_MINIT(electron),  /* PHP_MINIT - Module initialization */
    NULL,                 /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(electron),  /* PHP_RINIT - Request initialization */
    NULL,                 /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(electron),  /* PHP_MINFO - Module info */
    PHP_ELECTRON_VERSION, /* Version */
    STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_ELECTRON
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(electron)
#endif
