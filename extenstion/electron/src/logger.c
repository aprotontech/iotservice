/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     logger.c
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-10-28 11:03:13
 *  @version  0
 *  @brief
 *
 * *
 */

#include "common.h"
#include "proton/common/logger.h"

proton_private_value_t *_default_logger = NULL;

proton_logger_config_t *new_logger_config(int bufsize) {
  proton_logger_config_t *lg = (proton_logger_config_t *)qmalloc(
      sizeof(proton_logger_config_t) + bufsize);

  lg->buffer_size = bufsize;
  lg->callback = NULL;
  lg->file_split_size = 0;
  lg->min_level = PROTON_LOG_INFO_LEVEL;
  lg->print_to = 0;
  lg->skip_core_logger = 1;
  lg->stack_depth = 2;

  if (proton_logger_open(&lg->value) != 0) {
    qfree(lg);
    return NULL;
  }

  return lg;
}

/** {{{
 */
PHP_METHOD(logger, __construct) {
  long bufsize = 0;
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(bufsize)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  if (bufsize <= 0) {
    bufsize = PROTON_LOGGER_DEFAULT_BUFFER_SIZE;
  }

  proton_logger_config_t *lg = new_logger_config(bufsize);
  if (lg == NULL) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "init logger failed");
    return;
  }

  proton_object_construct(getThis(), &lg->value);
}
/* }}} */

/** {{{
 */
PHP_METHOD(logger, __destruct) {
  if (proton_object_get(getThis()) == _default_logger) {
    _default_logger = NULL;
  }
  proton_object_destruct(getThis());
}
/* }}} */

/** {{{
 */
PHP_METHOD(logger, __toString) { RETURN_STRING("{logger}"); }
/* }}} */

/** {{{
 */
PHP_METHOD(logger, getDefaultLogger) {

  if (_default_logger == NULL) {
    zval logger;
    object_init_ex(&logger, _logger_ce);

    proton_logger_config_t *lg =
        new_logger_config(PROTON_LOGGER_DEFAULT_BUFFER_SIZE);
    if (lg == NULL) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
                       "create default logger failed");
      return;
    }

    _default_logger = &lg->value;
    proton_object_construct(&logger, _default_logger);
  }

  RETURN_ZVAL(&_default_logger->myself, 0, 0);
}
/* }}} */

/** {{{
 */
PHP_METHOD(logger, setCoreLogger) {
  zval *logger = NULL;
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(logger, _logger_ce)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);
  RETURN_LONG(proton_logger_set_core(proton_object_get(logger)));
}
/* }}} */

/* {{{ Logger $logger.setLevel(int $level)
 */
PHP_METHOD(logger, setLevel) {
  long level;

  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(level)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  proton_logger_config_t *lg =
      (proton_logger_config_t *)proton_object_get(getThis());
  lg->min_level = level;

  RETURN_ZVAL(getThis(), 0, 0);
}
/* }}} */

/* {{{ void $logger.write( $log_level, $log_message[, $log_context] )
 */
PHP_METHOD(logger, write) {
  long level;
  char *message = NULL;
  size_t message_len;
  zval *ctx;
  proton_logger_config_t *lg =
      (proton_logger_config_t *)proton_object_get(getThis());

  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(level)
    Z_PARAM_STRING(message, message_len)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(ctx)
  ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

  int recall_depth = lg->stack_depth;
  const char *filename = NULL;
  const char *funcname = NULL;
  long code_line = 0;
  zend_execute_data *ptr = EG(current_execute_data);

  while (ptr != NULL && recall_depth >= 0) {
    if (ptr->prev_execute_data != NULL && ptr->prev_execute_data->func &&
        ZEND_USER_CODE(ptr->prev_execute_data->func->common.type)) {
      ptr = ptr->prev_execute_data;
    } else {
      break;
    }
    recall_depth--;
  }

  if (ptr != NULL && ptr->func != NULL && ZEND_USER_CODE(ptr->func->type)) {
    filename = ZSTR_VAL(ptr->func->op_array.filename);
    code_line = ptr->opline->lineno;
    funcname = ZSTR_VAL(ptr->func->op_array.function_name);
  }

  RETURN_LONG(proton_logger_write(proton_object_get(getThis()), level, 0,
                                  filename, code_line, filename, message));
}
/* }}}*/

/* {{{ int $logger.close()
 */
PHP_METHOD(logger, close) {

  RETURN_LONG(proton_logger_close(proton_object_get(getThis())));
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_proton_set_logger_level, 1)
  ZEND_ARG_INFO(0, enable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_proton_write_logger, 1)
  ZEND_ARG_INFO(0, fmt)
ZEND_END_ARG_INFO()

/* {{{ martin_functions[]
 *
 * Every user visible function must have an entry in logger_functions[].
 */
const zend_function_entry logger_functions[] = {
    PHP_ME(logger, __construct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_CTOR) // logger::__construct
    PHP_ME(logger, __destruct, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_DTOR) // logger::__destruct
    PHP_ME(logger, __toString, NULL,
           ZEND_ACC_PUBLIC) // logger::__toString
    PHP_ME(logger, write, NULL,
           ZEND_ACC_PUBLIC) // logger::write
    PHP_ME(logger, setLevel, NULL,
           ZEND_ACC_PUBLIC) // logger::set_level
    PHP_ME(logger, close, NULL,
           ZEND_ACC_PUBLIC) // logger::close
    PHP_ME(logger, setCoreLogger, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // logger::setCoreLogger
    PHP_ME(logger, getDefaultLogger, NULL,
           ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) // logger::getDefaultLogger
    {NULL, NULL, NULL} /* Must be the last line in logger_functions[] */
};
/* }}} */

zend_class_entry *_logger_ce;

zend_class_entry *regist_logger_class() {
  zend_class_entry logger;

  INIT_NS_CLASS_ENTRY(logger, PROTON_NAMESPACE, "Logger", logger_functions);
  _logger_ce = zend_register_internal_class_ex(&logger, NULL);

  zend_declare_property_null(_logger_ce, ZEND_STRL(PROTON_OBJECT_PRIVATE_VALUE),
                             ZEND_ACC_PRIVATE TSRMLS_CC);

  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_DEBUG"), PROTON_LOG_DEBUG_LEVEL);
  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_INFO"), PROTON_LOG_INFO_LEVEL);
  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_WARN"), PROTON_LOG_WARN_LEVEL);
  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_ERROR"), PROTON_LOG_ERROR_LEVEL);
  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_NOTICE"), PROTON_LOG_NOTICE_LEVEL);
  zend_declare_class_constant_long(
      _logger_ce, STRING_PARAM_EXPAND("LEVEL_FAULT"), PROTON_LOG_FAULT_LEVEL);

  return _logger_ce;
}