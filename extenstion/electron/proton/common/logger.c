/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     logger.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 11:41:36
 *
 */

#include "logger.h"

PROTON_TYPE_WHOAMI_DEFINE(_logger_get_type, "logger")

proton_logger_config_t *_default_core_logger = NULL;
static proton_value_type_t __proton_logger_type = {.construct = NULL,
                                                   .destruct =
                                                       proton_logger_close,
                                                   .whoami = _logger_get_type};

const char *__proton_logger_level_string(int level) {
  switch (level) {
  case PROTON_LOG_DEBUG_LEVEL:
    return "DEBUG";
  case PROTON_LOG_INFO_LEVEL:
    return "INFO";
  case PROTON_LOG_WARN_LEVEL:
    return "WARN";
  case PROTON_LOG_ERROR_LEVEL:
    return "ERROR";
  case PROTON_LOG_NOTICE_LEVEL:
    return "NOTICE";
  case PROTON_LOG_FAULT_LEVEL:
  default:
    return "FAULT";
  }
}

const char *_get_log_tag(proton_logger_config_t *config, int is_core) {
  if (config->skip_core_logger) {
    return "";
  }
  return is_core ? "[C] " : "[P] ";
}

int logger_format_prefix(proton_logger_config_t *config, int level, int is_core,
                         const char *file, int line, const char *func) {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  struct tm *p = localtime(&tv.tv_sec);

  return snprintf(config->buffer, config->buffer_size,
                  "%02d-%02d-%02d %02d:%02d:%02d.%03d %s[%d] [%s] [%s:%d] ",
                  p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour,
                  p->tm_min, p->tm_sec, (int)(tv.tv_usec / 1000),
                  _get_log_tag(config, is_core), getpid(),
                  __proton_logger_level_string(level), func, line);
}

int logger_write_to_file(proton_logger_config_t *config, int length) {
  puts(config->buffer);
  // printf("%s\n", config->buffer);

  config->writen_size += length;

  return length;
}

int proton_write_core_logger(int level, const char *file, int line,
                             const char *func, const char *fmt, ...) {
  proton_logger_config_t *config = _default_core_logger;
  if (config == NULL) {
    return RC_ERROR_INVALIDATE_INPUT;
  }

  if (config->skip_core_logger || level < config->min_level) {
    return 0;
  }

  int ret_prefix = logger_format_prefix(config, level, 1, file, line, func);

  va_list aptr;
  va_start(aptr, fmt);
  int ret_content = vsprintf(&config->buffer[0] + ret_prefix, fmt, aptr);
  // ret_content = vsnprintf(&(config->buffer[0]) + ret_prefix, 2048, fmt,
  // aptr);
  va_end(aptr);

  return logger_write_to_file(config, ret_prefix + ret_content);
}

int proton_logger_write(proton_private_value_t *value, int level, int is_core,
                        const char *file, int line, const char *func,
                        const char *fmt, ...) {
  proton_logger_config_t *config = (proton_logger_config_t *)value;
  if (config == NULL) {
    return RC_ERROR_INVALIDATE_INPUT;
  }

  if ((config->skip_core_logger && is_core) || level < config->min_level) {
    return 0;
  }

  int ret_prefix =
      logger_format_prefix(config, level, is_core, file, line, func);

  va_list aptr;
  va_start(aptr, fmt);

  int ret_content = vsnprintf(&config->buffer[0] + ret_prefix,
                              config->buffer_size - ret_prefix, fmt, aptr);
  va_end(aptr);

  return logger_write_to_file(config, ret_prefix + ret_content);
}

int proton_logger_open(proton_private_value_t *value) {
  proton_logger_config_t *config = (proton_logger_config_t *)value;
  config->writen_size = 0;

  config->value.type = &__proton_logger_type;
  ZVAL_UNDEF(&config->value.myself);
  return 0;
}

int proton_logger_close(proton_private_value_t *value) {
  if ((proton_logger_config_t *)value == _default_core_logger) {
    PLOG_INFO("current logger(%p) is using as core logger, so skip close it",
              value);
    // need more loggers, so don't free it
    // _default_core_logger = NULL;
    // Z_TRY_DELREF(_default_core_logger->value.myself);
    // ZVAL_UNDEF(&_default_core_logger->value.myself);
    return -1;
  }

  return 0;
}

int proton_logger_set_core(proton_private_value_t *config) {
  if (&_default_core_logger->value != config) {
    if (_default_core_logger != NULL) {
      RELEASE_VALUE_MYSELF(_default_core_logger->value);
      _default_core_logger = NULL;
    }
    _default_core_logger = (proton_logger_config_t *)config;
    _default_core_logger->skip_core_logger = 0;
    _default_core_logger->print_to |= 0x01;

    Z_TRY_ADDREF(_default_core_logger->value.myself);
  }

  return 0;
}

int proton_logger_global_cleanup() {
  if (_default_core_logger != NULL) { // do real cleanup
    PLOG_NOTICE("close core logger");
  }

  return 0;
}