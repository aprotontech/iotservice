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

typedef struct _proton_timer_t {
  uv_timer_t timer;
  proton_wait_object_t wq_tick; // timer queue
  proton_coroutine_runtime *runtime;
} proton_timer_t;

void on_timer_closed(uv_handle_t *handle) {
  proton_timer_t *pt = (proton_timer_t *)handle->data;

  qfree(pt);
}

void on_coroutine_sleep_done(uv_timer_t *timer) {
  PLOG_DEBUG("timer(%p) sleep done", timer);
  proton_timer_t *pt = (proton_timer_t *)timer->data;

  uv_timer_stop(timer);

  proton_coroutine_wakeup(pt->runtime, &pt->wq_tick, NULL);

  uv_close((uv_handle_t *)&pt->timer, on_timer_closed);
}

int proton_coroutine_sleep(proton_coroutine_runtime *runtime, long time_ms) {
  MAKESURE_ON_COROTINUE(runtime);
  proton_timer_t *pt = (proton_timer_t *)qmalloc(sizeof(proton_timer_t));
  pt->runtime = runtime;
  PROTON_WAIT_OBJECT_INIT(pt->wq_tick);

  uv_timer_init(RUNTIME_UV_LOOP(runtime), &pt->timer);
  pt->timer.data = pt;

  uv_timer_start(&pt->timer, on_coroutine_sleep_done, time_ms, 0);

  PLOG_INFO("coroutinue(%lu) sleep %dms",
            RUNTIME_CURRENT_COROUTINE(runtime)->cid, time_ms);

  proton_coroutine_waitfor(runtime, &pt->wq_tick, NULL);

  return 0;
}