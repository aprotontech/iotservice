/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     logger.h
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-10-28 11:07:56
 *  @version  0
 *  @brief
 *
 * *
 */

#ifndef _PROTON_ELECTRON_LOGGER_H_
#define _PROTON_ELECTRON_LOGGER_H_

#include "electron.h"

typedef struct _proton_logger_config_t {
  proton_private_value_t value;
  int min_level;
  int skip_core_logger;

  int stack_depth;

  int print_to;

  int64_t writen_size;

  int64_t file_split_size;

  zval *callback;

  // last members
  int buffer_size;
  char buffer[4];
} proton_logger_config_t;

int proton_logger_open(proton_private_value_t *config);

int proton_logger_close(proton_private_value_t *config);

int proton_logger_write(proton_private_value_t *config, int level, int is_core,
                        const char *file, int line, const char *func,
                        const char *fmt, ...);

int proton_logger_set_core(proton_private_value_t *config);

int proton_logger_global_cleanup();

#endif