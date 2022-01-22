/* electron extension for PHP */

#include "src/common.h"

static int pc_resource_handle;
int pc_private_resource_handle;

static void destruct_proton_coroutine(zend_resource *rsrc) {
  QUARK_LOGGER("[ELECTRON] coroutinue destruct");
  quark_coroutine_task *task = (quark_coroutine_task *)rsrc->ptr;

  quark_coroutine_destory(task);
}

/* {{{ void quark_coroutine_yield(resource $qc)
 */
PHP_FUNCTION(quark_enable_logger) {
  long enable, tmp;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(enable)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  tmp = __quark_logger;
  __quark_logger = enable;
  RETURN_LONG(tmp);
}
/* }}} */

extern zend_class_entry *regist_runtime_class();
extern zend_class_entry *regist_tcpserver_class();
extern zend_class_entry *regist_tcpclient_class();
extern zend_class_entry *regist_coroutine_class();
extern zend_class_entry *regist_httpserver_class();
extern zend_class_entry *regist_httpclient_class();
PHP_MINIT_FUNCTION(electron) {
  pc_resource_handle = zend_register_list_destructors_ex(
      destruct_proton_coroutine, NULL, PHP_COROUTINE_RESOURCE_NAME,
      module_number);
  if (pc_resource_handle == FAILURE) {
    QUARK_LOGGER("regist resource %s failed\n", PHP_COROUTINE_RESOURCE_NAME);
  }

  pc_private_resource_handle = zend_register_list_destructors_ex(
      destruct_proton_private_value, NULL, PHP_PRIVATE_VALUE_RESOURCE_NAME,
      module_number);
  if (pc_private_resource_handle == FAILURE) {
    QUARK_LOGGER("regist resource %s failed\n",
                 PHP_PRIVATE_VALUE_RESOURCE_NAME);
  }

  regist_runtime_class();
  regist_tcpserver_class();
  regist_tcpclient_class();
  regist_coroutine_class();
  regist_httpserver_class();
  regist_httpclient_class();
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
ZEND_BEGIN_ARG_INFO(arginfo_quark_enable_logger, 1)
  ZEND_ARG_INFO(0, enable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_yield, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_sleep, 1)
  ZEND_ARG_INFO(0, time_ms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quark_coroutine_create, 1)
  ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_context, 0)
ZEND_END_ARG_INFO()
/* }}} */

extern PHP_FUNCTION(proton_go);
extern PHP_FUNCTION(proton_context);
extern PHP_FUNCTION(proton_yield);
extern PHP_FUNCTION(proton_sleep);
extern PHP_FUNCTION(proton_runtime_start);
extern PHP_FUNCTION(proton_runtime_stop);

/* {{{ electron_functions[]
 */
static const zend_function_entry electron_functions[] = {
    PHP_FE(quark_enable_logger, arginfo_quark_enable_logger) // logger

    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, sleep, ZEND_FN(proton_sleep),
                     arginfo_quark_coroutine_sleep) // proton::sleep
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, pause, ZEND_FN(proton_yield),
                     arginfo_quark_coroutine_yield) // proton::yield
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, go, ZEND_FN(proton_go),
                     arginfo_quark_coroutine_create) // proton::go
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, context, ZEND_FN(proton_context),
                     arginfo_proton_context) // proton::go
    PHP_FE_END                               // eof
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
