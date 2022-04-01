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

#define MAKESURE_ON_COROTINUE(runtime)                                         \
  if (RUNTIME_CURRENT_COROUTINE(runtime) == RUNTIME_MAIN_COROUTINE(runtime)) { \
    PLOG_WARN("%s only can run on coroutine", __FUNCTION__);                   \
    return RC_ERROR_NOT_REAL_COROUTINUE;                                       \
  }

typedef struct _proton_coroutine_runtime_t {
  proton_private_value_t value;

  list_link_t clist;

  // task can be released
  list_link_t releaseables;

  list_link_t runables;

  void *data; // custom data --> scheduler loop

  zval error_handler;

  // current running coroutine
  proton_coroutine_task *current;

  // main coroutine
  proton_coroutine_task main;
} proton_coroutine_runtime;

typedef struct _proton_wait_object_t proton_wait_object_t;
typedef int (*proton_wait_cancel)(proton_wait_object_t *value);

typedef enum _proton_wait_mode {
  QC_MODE_FIFO = 0,
  QC_MODE_ALL = 1,
} proton_wait_mode;

typedef struct _proton_wait_object_t {
  // link to [proton_coroutine_task.waiting]
  list_link_t head;
  proton_wait_mode mode;

  // TODO: support later
  proton_wait_cancel cancel;
  int is_canceled;
} proton_wait_object_t;

#define PROTON_WAIT_OBJECT_INIT(obj)                                           \
  {                                                                            \
    (obj).mode = QC_MODE_FIFO;                                                 \
    (obj).cancel = NULL;                                                       \
    (obj).is_canceled = 0;                                                     \
    LL_init(&((obj).head));                                                    \
  }

#define IS_COROUTINE_WAITFOR(obj) (!LL_isspin(&((obj).head)))

///////////// RUNTIME
proton_coroutine_runtime *
proton_runtime_init(proton_coroutine_runtime *runtime);

int proton_runtime_uninit(proton_coroutine_runtime *runtime);

int proton_coroutine_schedule(proton_coroutine_runtime *runtime);

int proton_coroutine_notify_reschedule(proton_coroutine_runtime *runtime);

///////////// COROUTINE

proton_coroutine_task *
proton_coroutine_create(proton_coroutine_runtime *runtime,
                        proton_coroutine_entry *entry, int c_stack_size,
                        int php_stack_size);

int proton_coroutine_yield(proton_coroutine_runtime *runtime,
                           proton_coroutine_task **task);

int proton_coroutine_resume(proton_coroutine_runtime *runtime,
                            proton_coroutine_task *task);

int proton_coroutine_waitfor(proton_coroutine_runtime *runtime,
                             proton_wait_object_t *value,
                             proton_coroutine_task **task);

int proton_coroutine_wakeup(proton_coroutine_runtime *runtime,
                            proton_wait_object_t *value,
                            proton_coroutine_task **task);

int proton_coroutine_cancel(proton_coroutine_runtime *runtime,
                            proton_wait_object_t *value,
                            proton_coroutine_task **task);

#endif