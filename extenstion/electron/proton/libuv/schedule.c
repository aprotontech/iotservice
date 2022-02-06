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

proton_uv_scheduler *proton_scheduler_init(proton_uv_scheduler *scheduler) {
  if (scheduler != NULL && scheduler->loop == NULL) {
    PLOG_DEBUG("scheduler init");
    scheduler->loop = uv_loop_new();
    scheduler->loop->data = &scheduler->runtime;

    proton_runtime_init(&scheduler->runtime);

    scheduler->runtime.data = scheduler->loop;

    uv_idle_init(scheduler->loop, &scheduler->idle);
    scheduler->idle.data = scheduler;
    uv_idle_start(&scheduler->idle, coroutine_schedule);
  }
  return scheduler;
}

proton_uv_scheduler *proton_scheduler_create() {
  // can't use pmalloc here, because scheduler is global object
  proton_uv_scheduler *s =
      (proton_uv_scheduler *)malloc(sizeof(proton_uv_scheduler));
  memset(s, 0, sizeof(proton_uv_scheduler));

  return proton_scheduler_init(s);
}

int proton_scheduler_uninit(proton_uv_scheduler *scheduler) { return 0; }

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
  return 0;
}

int proton_scheduler_free(proton_uv_scheduler *scheduler) {
  PLOG_DEBUG("free scheduler");
  return 0;
}