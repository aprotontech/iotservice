/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     runtime.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 11:39:28
 *
 */

#ifndef __QUARK_LIBUV_RUNTIME_H__
#define __QUARK_LIBUV_RUNTIME_H__

#include "proton/common/electron.h"
#include "proton/coroutine/task.h"

#define RUNTIME_CURRENT_COROUTINE(runtime)                                     \
  (((/*(proton_coroutine_runtime *)*/ (runtime)))->current)

#define RUNTIME_MAIN_COROUTINE(runtime)                                        \
  (&(((/*(proton_coroutine_runtime *)*/ (runtime)))->main))

#define IS_REAL_COROUTINE(this_coroutine)                                      \
  ((this_coroutine != NULL) && ((this_coroutine)->runtime != NULL) &&          \
   ((this_coroutine) != RUNTIME_MAIN_COROUTINE((this_coroutine)->runtime)))

typedef struct _proton_coroutine_runtime_t {
  proton_private_value_t value;

  list_link_t clist;

  list_link_t runables;

  void *data; // custom data --> scheduler loop

  // current running coroutine
  proton_coroutine_task *current;

  // main coroutine
  proton_coroutine_task main;
} proton_coroutine_runtime;

proton_coroutine_runtime *
proton_runtime_init(proton_coroutine_runtime *runtime);

int quark_runtime_loop(proton_coroutine_runtime *runtime);
int quark_runtime_stop(proton_coroutine_runtime *runtime);

proton_coroutine_task *
proton_coroutine_create(proton_coroutine_runtime *runtime,
                        proton_coroutine_entry *entry, int c_stack_size,
                        int php_stack_size);

int proton_coroutine_sleep(proton_coroutine_runtime *runtime, long time_ms);
int proton_coroutine_yield(proton_coroutine_runtime *runtime);
int proton_coroutine_waitfor(proton_coroutine_runtime *runtime,
                             proton_private_value_t *value);
int proton_coroutine_resume(proton_coroutine_runtime *runtime,
                            proton_coroutine_task *task);

int proton_coroutine_schedule(proton_coroutine_runtime *runtime);

#endif