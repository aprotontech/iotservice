/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     timer.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 11:46:43
 *
 */

#include "runtime.h"
#include "proton/coroutine/task.h"

void on_coroutine_sleep_done(uv_timer_t *timer) {
  quark_coroutine_task *task = (quark_coroutine_task *)timer->data;

  quark_coroutine_swap_in(task);

  uv_timer_stop(timer);

  qfree(timer);
}

int quark_coroutine_sleep(quark_coroutine_runtime *runtime, long time_ms) {
  quark_coroutine_task *task = quark_coroutine_current();
  uv_timer_t *timer = (uv_timer_t *)qmalloc(sizeof(uv_timer_t));
  uv_timer_init(runtime->loop, timer);
  timer->data = task;

  uv_timer_start(timer, on_coroutine_sleep_done, time_ms, 0);

  quark_coroutine_swap_out(task, QC_STATUS_SUSPEND);

  return 0;
}