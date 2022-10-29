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
#include "errcode.h"
#include "http_parser.h"

#include "php.h"
#include "ext/standard/info.h"

#define qmalloc emalloc
#define qfree efree

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

#define RELEASE_VALUE_MYSELF(value) ZVAL_PTR_DTOR((&((value).myself)))

#define RC_EXPECT_SUCCESS_WITH_EMSG(expr, message)                             \
  {                                                                            \
    int _rc = expr;                                                            \
    if (_rc != 0) {                                                            \
      PLOG_WARN("except (%s) success, but faile with %d", message, _rc);       \
      return _rc;                                                              \
    }                                                                          \
  }

#define RC_EXPECT_SUCCESS(expr) RC_EXPECT_SUCCESS_WITH_EMSG(expr, #expr)

////////////////////////////////////////////
// Logger
#define PROTON_LOG_DEBUG_LEVEL 0
#define PROTON_LOG_INFO_LEVEL 1
#define PROTON_LOG_WARN_LEVEL 2
#define PROTON_LOG_ERROR_LEVEL 3
#define PROTON_LOG_NOTICE_LEVEL 4
#define PROTON_LOG_FAULT_LEVEL 5

#define PLOG_DEBUG(fmt, args...)                                               \
  proton_write_core_logger(PROTON_LOG_DEBUG_LEVEL, __FILE__, __LINE__,         \
                           __FUNCTION__, fmt, ##args)
#define PLOG_INFO(fmt, args...)                                                \
  proton_write_core_logger(PROTON_LOG_INFO_LEVEL, __FILE__, __LINE__,          \
                           __FUNCTION__, fmt, ##args)
#define PLOG_WARN(fmt, args...)                                                \
  proton_write_core_logger(PROTON_LOG_WARN_LEVEL, __FILE__, __LINE__,          \
                           __FUNCTION__, fmt, ##args)
#define PLOG_ERROR(fmt, args...)                                               \
  proton_write_core_logger(PROTON_LOG_ERROR_LEVEL, __FILE__, __LINE__,         \
                           __FUNCTION__, fmt, ##args)
#define PLOG_NOTICE(fmt, args...)                                              \
  proton_write_core_logger(PROTON_LOG_NOTICE_LEVEL, __FILE__, __LINE__,        \
                           __FUNCTION__, fmt, ##args)
#define PLOG_FAULT(fmt, args...)                                               \
  proton_write_core_logger(PROTON_LOG_FAULT_LEVEL, __FILE__, __LINE__,         \
                           __FUNCTION__, fmt, ##args)

int proton_write_core_logger(int level, const char *file, int line,
                             const char *func, const char *fmt, ...);
////////////////////////////////////////////

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
#ifdef PROTON_DEBUG_TRACE_OBJECT
  list_link_t link;
#endif
} proton_private_value_t;

#define PROTON_TYPE_WHOAMI_DEFINE(func_name, type_name)                        \
  const char *func_name() { return type_name; }

extern list_link_t *__all_values;

#ifdef PROTON_DEBUG_TRACE_OBJECT
#define PROTON_TRACE_VALUE(value) LL_insert(&value.link, __all_values->prev)
#define PROTON_REMOVE_TRACE(value) LL_remove(&value.link)
#else
#define PROTON_TRACE_VALUE(value)
#endif
#endif