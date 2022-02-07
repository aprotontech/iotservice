/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     debug.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-06 10:16:31
 *
 */

#include "electron.h"

list_link_t *__all_values = NULL;

int proton_values_trace_init() {
  if (__all_values == NULL) {
    __all_values = (list_link_t *)malloc(sizeof(list_link_t));
    LL_init(__all_values);
  }
  return 0;
}

int proton_values_trace_uninit() {
  if (__all_values != NULL) {
    free(__all_values);
    __all_values = NULL;
  }
}