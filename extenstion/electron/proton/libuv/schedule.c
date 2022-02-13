/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     schedule.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-30 11:04:46
 *
 */

#include "uvobject.h"

static void coroutine_schedule(uv_idle_t *handle) {
  proton_uv_scheduler *scheduler = (proton_uv_scheduler *)handle->data;
  if (scheduler != NULL) {
    proton_coroutine_schedule(&scheduler->runtime);
  }
}

void coroutine_schedule_tick(uv_timer_t *handle) {
  proton_uv_scheduler *scheduler = (proton_uv_scheduler *)handle->data;
  if (scheduler != NULL) {
    proton_coroutine_schedule(&scheduler->runtime);
  }
}

int proton_scheduler_init(proton_uv_scheduler *scheduler) {
  MAKESURE_PTR_NOT_NULL(scheduler);
  PLOG_DEBUG("scheduler init");
  uv_loop_init(&scheduler->loop);
  scheduler->loop.data = &scheduler->runtime;

  proton_runtime_init(&scheduler->runtime);

  scheduler->runtime.data = &scheduler->loop;

  // uv_idle_init(&scheduler->loop, &scheduler->idle);
  // scheduler->idle.data = scheduler;
  // uv_idle_start(&scheduler->idle, coroutine_schedule);

  uv_timer_init(&scheduler->loop, &scheduler->schedule_timer);
  scheduler->schedule_timer.data = scheduler;
  uv_timer_start(&scheduler->schedule_timer, coroutine_schedule_tick, 1, 1);

  return 0;
}

proton_uv_scheduler *proton_scheduler_create() {
  // can't use pmalloc here, because scheduler is global object
  proton_uv_scheduler *s =
      (proton_uv_scheduler *)malloc(sizeof(proton_uv_scheduler));
  memset(s, 0, sizeof(proton_uv_scheduler));

  proton_scheduler_init(s);
  return s;
}

int proton_scheduler_uninit(proton_uv_scheduler *scheduler) {
  MAKESURE_PTR_NOT_NULL(scheduler);

  uv_timer_stop(&scheduler->schedule_timer);

  uv_loop_close(&scheduler->loop);

  proton_runtime_uninit(&scheduler->runtime);

  return 0;
}

int proton_runtime_loop(proton_coroutine_runtime *runtime) {
  MAKESURE_PTR_NOT_NULL(runtime);

  if (RUNTIME_CURRENT_COROUTINE(runtime) != RUNTIME_MAIN_COROUTINE(runtime)) {
    PLOG_WARN("[RUNTIME] loop only run on main coroutinue");
    return -1;
  }

  PLOG_INFO("start runtime uvloop");
  return uv_run((uv_loop_t *)runtime->data, UV_RUN_DEFAULT);
}

int proton_runtime_stop(proton_coroutine_runtime *runtime) {
  if (runtime == NULL) {
    PLOG_WARN("input runtime is null");
    return -1;
  }

  PLOG_INFO("stop runtime uvloop");
  uv_stop(RUNTIME_UV_LOOP(runtime));

  // do last schedule
  // proton_coroutine_schedule(runtime);

  return 0;
}

int proton_scheduler_free(proton_uv_scheduler *scheduler) {
  PLOG_DEBUG("free scheduler");
  if (proton_scheduler_uninit(scheduler) == 0) {
    free(scheduler);
  }
  return 0;
}