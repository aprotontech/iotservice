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

#include "uvobject.h"
#include "proton/coroutine/task.h"

void on_coroutine_sleep_done(uv_timer_t *timer) {
  PLOG_INFO("timer sleep done");
  proton_coroutine_task *task = (proton_coroutine_task *)timer->data;

  uv_timer_stop(timer);
  task->status = QC_STATUS_RUNABLE;
  proton_coroutine_resume(NULL, task);

  qfree(timer);
}

int proton_coroutine_sleep(proton_coroutine_runtime *runtime, long time_ms) {
  MAKESURE_ON_COROTINUE(runtime);
  proton_coroutine_task *task = RUNTIME_CURRENT_COROUTINE(runtime);
  uv_timer_t *timer = (uv_timer_t *)qmalloc(sizeof(uv_timer_t));
  uv_timer_init(RUNTIME_UV_LOOP(runtime), timer);
  timer->data = task;

  uv_timer_start(timer, on_coroutine_sleep_done, time_ms, 0);

  proton_coroutine_waitfor(runtime, NULL);

  return 0;
}