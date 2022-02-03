/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     variable.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-30 09:47:01
 *
 */

#ifndef _PROTON_COROUTINE_VARIABLE_H_
#define _PROTON_COROUTINE_VARIABLE_H_

#include "proton/common/electron.h"

typedef struct _proton_coroutine_task proton_coroutine_task;

typedef enum _proton_coroutine_var_mode {
  QC_VAR_SHARE = 0,
  QC_VAR_READONLY,
  QC_VAR_INHERIT,
} proton_coroutine_var_mode;

typedef struct _proton_coroutine_var_t {
  zval val;
  proton_coroutine_var_mode mode;
  char set_by_me;
  char key[4];
} proton_coroutine_var_t;

int proton_coroutine_get_var(proton_coroutine_task *current, const char *key,
                             zval *val);

int proton_coroutine_set_var(proton_coroutine_task *current, const char *key,
                             zval *val, proton_coroutine_var_mode mode);

#endif