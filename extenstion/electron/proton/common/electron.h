/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     electron.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 11:32:30
 *
 */

#ifndef __QUARK_ELECTRON_H__
#define __QUARK_ELECTRON_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "clist.h"
#include "hashmap.h"
#include "http_parser.h"

#include "php.h"
#include "ext/standard/info.h"

#define qmalloc emalloc
#define qfree efree

#define PROTON_LOG_DEBUG_LEVEL 0
#define PROTON_LOG_INFO_LEVEL 1
#define PROTON_LOG_WARN_LEVEL 2
#define PROTON_LOG_ERROR_LEVEL 3
#define PROTON_LOG_NOTICE_LEVEL 4
#define PROTON_LOG_FAULT_LEVEL 5

#define PLOG_DEBUG(args...) PROTON_LOGGER(PROTON_LOG_DEBUG_LEVEL, args)
#define PLOG_INFO(args...) PROTON_LOGGER(PROTON_LOG_INFO_LEVEL, args)
#define PLOG_WARN(args...) PROTON_LOGGER(PROTON_LOG_WARN_LEVEL, args)
#define PLOG_ERROR(args...) PROTON_LOGGER(PROTON_LOG_ERROR_LEVEL, args)
#define PLOG_NOTICE(args...) PROTON_LOGGER(PROTON_LOG_NOTICE_LEVEL, args)
#define PLOG_FAULT(args...) PROTON_LOGGER(PROTON_LOG_FAULT_LEVEL, args)

#define PROTON_LOGGER(level, args...)                                          \
  if (level >= __proton_logger_level) {                                        \
    printf("[C] [%s] [%s:%d] ", __proton_logger_level_string(level),           \
           __FUNCTION__, __LINE__);                                            \
    printf(args);                                                              \
    printf("\n");                                                              \
  }

extern int __proton_logger_level;
extern const char *__proton_logger_level_string(int level);

#ifndef container_of
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    const typeof(((type *)0)->member) *__mptr = (ptr);                         \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })
#endif

#define MAKESURE_PTR_NOT_NULL(ptr)                                             \
  if ((ptr) == NULL) {                                                         \
    PLOG_WARN("(%s) must not be null", #ptr);                                  \
    return 100;                                                                \
  }

#define RELEASE_VALUE_MYSELF(value) ZVAL_PTR_DTOR((&value.myself))

typedef struct _proton_private_value_t proton_private_value_t;
typedef void (*proton_value_new)(proton_private_value_t *value);
typedef void (*proton_value_del)(proton_private_value_t *value);

typedef struct _proton_private_value_t {
  int type;
  zval myself;
} proton_private_value_t;

#endif