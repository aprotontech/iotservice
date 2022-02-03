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

#include "electron.h"

int __proton_logger_level = PROTON_LOG_FAULT_LEVEL + 1;
zval *__quark_logger_callback;

#define PROTON_LOG_DEBUG_LEVEL 0
#define PROTON_LOG_INFO_LEVEL 1
#define PROTON_LOG_WARN_LEVEL 2
#define PROTON_LOG_ERROR_LEVEL 3
#define PROTON_LOG_NOTICE_LEVEL 4
#define PROTON_LOG_FAULT_LEVEL 5

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

int quark_logger(int level, const char *file, int line, const char *func,
                 const char *fmt, ...) {

  return 0;
}