/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     manager.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-14 10:42:59
 *
 */

#include "runtime.h"
#include "proton/coroutine/task.h"

static quark_coroutine_runtime *__runtime;

void coroutine_schedule(uv_idle_t *handle);

quark_coroutine_runtime *quark_get_runtime() {
  if (__runtime == NULL) {
    __runtime =
        (quark_coroutine_runtime *)emalloc(sizeof(quark_coroutine_runtime));
    LL_init(&__runtime->clist);
    LL_init(&__runtime->runables);
    __runtime->cmap = hashmap_new();

    __runtime->loop = uv_default_loop();
    __runtime->loop->data = __runtime;

    uv_idle_init(__runtime->loop, &__runtime->idle);
    __runtime->idle.data = __runtime;
    uv_idle_start(&__runtime->idle, coroutine_schedule);

    quark_coroutine_env_init();
  }
  return __runtime;
}

quark_coroutine_task *quark_runtime_main(quark_coroutine_runtime *runtime) {
  return quark_coroutine_get_main();
}

int quark_runtime_loop(quark_coroutine_runtime *runtime) {
  if (runtime == NULL) {
    QUARK_LOGGER("input runtime is null");
    return -1;
  }

  if (quark_coroutine_get_main() != quark_coroutine_current()) {
    QUARK_LOGGER("[RUNTIME] loop only run on main coroutinue");
    return -1;
  }
  return uv_run(runtime->loop, UV_RUN_DEFAULT);
}

int quark_runtime_stop(quark_coroutine_runtime *runtime) {
  if (runtime == NULL) {
    QUARK_LOGGER("input runtime is null");
    return -1;
  }
  uv_stop(runtime->loop);
  return 0;
}

int quark_coroutine_yield(quark_coroutine_runtime *runtime) {
  quark_coroutine_task *task = quark_coroutine_current();

  LL_remove(&task->link);
  LL_insert(&task->link, runtime->runables.prev);
  return quark_coroutine_swap_out(task, QC_STATUS_RUNABLE);
}

int quark_coroutine_ready(quark_coroutine_runtime *runtime) { return 0; }

void coroutine_schedule(uv_idle_t *handle) {
  quark_coroutine_runtime *runtime = handle->data;

  if (!LL_isspin(&runtime->runables)) {
    list_link_t *p = runtime->runables.next;
    LL_remove(runtime->runables.next);
    quark_coroutine_task *task = container_of(p, quark_coroutine_task, link);
    if (task == NULL) {
      QUARK_LOGGER("invalidate task from runable link");
      return;
    }

    quark_coroutine_swap_in(task);
  }
}