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
    struct timeval tv;                                                         \
    struct timezone tz;                                                        \
    gettimeofday(&tv, &tz);                                                    \
    struct tm *p = localtime(&tv.tv_sec);                                      \
    printf("%02d-%02d-%02d %02d:%02d:%02d.%03d [C] [%s] [%s:%d] ",             \
           p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour,           \
           p->tm_min, p->tm_sec, (int)(tv.tv_usec / 1000),                     \
           __proton_logger_level_string(level), __FUNCTION__, __LINE__);       \
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
typedef int (*proton_value_new)(proton_private_value_t *value);
typedef int (*proton_value_del)(proton_private_value_t *value);
typedef const char *(*proton_value_who)();

typedef struct _proton_value_type_t {
  proton_value_who whoami;
  proton_value_new construct;
  proton_value_del destruct;
} proton_value_type_t;

typedef struct _proton_private_value_t {
  proton_value_type_t *type;
  zval myself;
} proton_private_value_t;

#define PROTON_TYPE_WHOAMI_DEFINE(func_name, type_name)                        \
  const char *func_name() { return type_name; }

#endif