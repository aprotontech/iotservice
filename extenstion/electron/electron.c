/* electron extension for PHP */

#include "src/common.h"

int pc_private_resource_handle;
proton_uv_scheduler *__uv_scheduler;

/* {{{ int proton_set_logger_level(int $level)
 */
PHP_FUNCTION(proton_set_logger_level) {
  long enable, tmp;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(enable)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  tmp = __proton_logger_level;
  __proton_logger_level = enable;
  RETURN_LONG(tmp);
}
/* }}} */

/* {{{ void proton_printf(...$args)
 */
PHP_FUNCTION(proton_printf) { RETURN_TRUE; }
/* }}} */

extern zend_class_entry *regist_runtime_class();
extern zend_class_entry *regist_channel_class();
extern zend_class_entry *regist_coroutine_class();
extern zend_class_entry *regist_tcpserver_class();
extern zend_class_entry *regist_tcpclient_class();
extern zend_class_entry *regist_httpserver_class();
extern zend_class_entry *regist_httpconnect_class();
extern zend_class_entry *regist_httpclient_class();
extern zend_class_entry *regist_httprequest_class();
extern zend_class_entry *regist_httpresponse_class();
extern zend_class_entry *regist_mqttclient_class();
extern zend_class_entry *regist_fsevent_class();
extern zend_class_entry *regist_processgroup_class();
PHP_MINIT_FUNCTION(electron) {

  pc_private_resource_handle = zend_register_list_destructors_ex(
      destruct_proton_private_value, NULL, PHP_PRIVATE_VALUE_RESOURCE_NAME,
      module_number);
  if (pc_private_resource_handle == FAILURE) {
    PLOG_ERROR("regist resource %s failed\n", PHP_PRIVATE_VALUE_RESOURCE_NAME);
  }

  __uv_scheduler = proton_scheduler_create();
  if (__uv_scheduler == NULL) {
    PLOG_ERROR("create proton uv-scheduler failed");
  }

  regist_runtime_class();
  regist_channel_class();
  regist_tcpserver_class();
  regist_tcpclient_class();
  regist_coroutine_class();
  regist_httpserver_class();
  regist_httpconnect_class();
  regist_httpclient_class();
  regist_httprequest_class();
  regist_httpresponse_class();
  regist_mqttclient_class();
  regist_fsevent_class();
  regist_processgroup_class();
}

/* {{{ PHP_MSHUTDOWN
 */
PHP_MSHUTDOWN_FUNCTION(electron) {
  if (__uv_scheduler != NULL) {
    if (proton_scheduler_free(__uv_scheduler) != 0) {
      PLOG_ERROR("uninit proton scheduler failed");
    }
    __uv_scheduler = NULL;
  }
}
/* }}} */

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
ZEND_BEGIN_ARG_INFO(arginfo_proton_set_logger_level, 1)
  ZEND_ARG_INFO(0, enable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_printf, 1)
  ZEND_ARG_INFO(0, fmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_coroutine_yield, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_coroutine_sleep, 1)
  ZEND_ARG_INFO(0, time_ms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_coroutine_create, 1)
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
    PHP_FE(proton_set_logger_level, arginfo_proton_set_logger_level) // logger
    PHP_FE(proton_printf, arginfo_proton_printf) // proton_printf

    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, sleep, ZEND_FN(proton_sleep),
                     arginfo_proton_coroutine_sleep) // proton::sleep
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, pause, ZEND_FN(proton_yield),
                     arginfo_proton_coroutine_yield) // proton::yield
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, go, ZEND_FN(proton_go),
                     arginfo_proton_coroutine_create) // proton::go
    ZEND_NS_NAMED_FE(PROTON_NAMESPACE, context, ZEND_FN(proton_context),
                     arginfo_proton_context) // proton::go
    PHP_FE_END                               // eof
};
/* }}} */

/* {{{ electron_module_entry
 */
zend_module_entry electron_module_entry = {
    STANDARD_MODULE_HEADER,
    "electron",              /* Extension name */
    electron_functions,      /* zend_function_entry */
    PHP_MINIT(electron),     /* PHP_MINIT - Module initialization */
    PHP_MSHUTDOWN(electron), /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(electron),     /* PHP_RINIT - Request initialization */
    NULL,                    /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(electron),     /* PHP_MINFO - Module info */
    PHP_ELECTRON_VERSION,    /* Version */
    STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_ELECTRON
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(electron)
#endif
