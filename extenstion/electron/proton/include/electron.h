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

#define QUARK_LOGGER(args...)                                                  \
  if (__quark_logger) {                                                        \
    php_printf(args);                                                          \
    php_printf("\n");                                                          \
  }

#define qmalloc emalloc
#define qfree efree

#ifndef container_of
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    const typeof(((type *)0)->member) *__mptr = (ptr);                         \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })
#endif

extern int __quark_logger;

#endif