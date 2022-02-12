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

extern int proton_coroutine_swap_in(proton_coroutine_task *dest);
extern int proton_coroutine_swap_out(proton_coroutine_task *current,
                                     proton_coroutine_status new_status);

extern void save_vm_stack(proton_coroutine_task *task);
extern proton_coroutine_task *
_proton_coroutine_create(proton_coroutine_task *current,
                         proton_coroutine_entry *entry, int c_stack_size,
                         int php_stack_size);

proton_coroutine_runtime *
proton_runtime_init(proton_coroutine_runtime *runtime) {
  if (runtime != NULL) {

    memset(runtime, 0, sizeof(proton_coroutine_runtime));

    LL_init(&runtime->clist);
    LL_init(&runtime->runables);
    LL_init(&runtime->releaseables);

    // init main coroutine
    LL_init(&runtime->main.link);
    LL_init(&runtime->main.runable);
    LL_init(&runtime->main.waiting);

    runtime->main.runtime = runtime;
    runtime->main.status = QC_STATUS_RUNNING;

    save_vm_stack(&runtime->main);

    runtime->current = &runtime->main;
  }

  return runtime;
}

proton_coroutine_task *
proton_coroutine_create(proton_coroutine_runtime *runtime,
                        proton_coroutine_entry *entry, int c_stack_size,
                        int php_stack_size) {
  if (c_stack_size <= 0) {
    c_stack_size = DEFAULT_C_STACK_SIZE;
  }
  if (php_stack_size <= 0) {
    php_stack_size = DEFAULT_PHP_STACK_PAGE_SIZE;
  }

  proton_coroutine_task *task = _proton_coroutine_create(
      RUNTIME_CURRENT_COROUTINE(runtime), entry, c_stack_size, php_stack_size);
  if (task != NULL) {
    // append to runable task list
    task->status = QC_STATUS_RUNABLE;
    LL_insert(&task->runable, runtime->runables.prev);
    LL_insert(&task->link, runtime->clist.prev);
    task->runtime = runtime;

    return task;
  }

  return NULL;
}

int proton_coroutine_yield(proton_coroutine_runtime *runtime,
                           proton_coroutine_task **out_task) {
  MAKESURE_PTR_NOT_NULL(runtime);
  proton_coroutine_task *task = RUNTIME_CURRENT_COROUTINE(runtime);

  if (!LL_isspin(&task->runable)) { // it must be spin
    PLOG_ERROR("current coroutine[%ld] is running, but it masked as runable",
               task->cid);
    LL_remove(&task->runable);
  }

  LL_insert(&task->runable, runtime->runables.prev);

  int rc = proton_coroutine_swap_out(task, QC_STATUS_RUNABLE);
  if (rc == 0 && out_task != NULL) {
    *out_task = task;
  }

  return rc;
}

int proton_coroutine_resume(proton_coroutine_runtime *runtime,
                            proton_coroutine_task *task) {
  MAKESURE_PTR_NOT_NULL(task);
  if (runtime == NULL) {
    runtime = task->runtime;
  } else if (runtime != task->runtime) {
    PLOG_WARN("input runtime is not match task's runtime");
    return 101;
  }

  MAKESURE_PTR_NOT_NULL(runtime);

  if (task->status != QC_STATUS_RUNABLE) {
    PLOG_ERROR(
        "coroutine[%ld] is not runable, current status is %d, can't resume",
        task->cid, task->status);
    return -1;
  }

  if (LL_isspin(&task->runable)) {
    PLOG_ERROR("coroutine[%ld] status is invalidate, runable link is spin",
               task->cid);
  }

  LL_remove(&task->runable);
  return proton_coroutine_swap_in(task);
}

int proton_coroutine_waitfor(proton_coroutine_runtime *runtime,
                             proton_wait_object_t *value,
                             proton_coroutine_task **out_task) {
  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(value);

  proton_coroutine_task *task = RUNTIME_CURRENT_COROUTINE(runtime);

  if (!LL_isspin(&task->waiting)) {
    PLOG_WARN("coroutine[%ld] status is invalidate, is waiting now", task->cid);
    return -1;
  }

  // insert to wait queue
  LL_insert(&task->waiting, value->head.prev);

  int rc = proton_coroutine_swap_out(task, QC_STATUS_SUSPEND);
  if (rc == 0) {
    if (out_task != NULL) {
      *out_task = task;
    }
  } else { // swap failed, so remove waiting link
    LL_remove(&task->waiting);
  }

  return rc;
}

int proton_coroutine_wakeup(proton_coroutine_runtime *runtime,
                            proton_wait_object_t *value,
                            proton_coroutine_task **out_task) {
  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(value);

  if (LL_isspin(&value->head)) {
    PLOG_WARN("not found any coroutine to wake up");
    return -1;
  }

  proton_coroutine_task *task = NULL;
  switch (value->mode) { // only support FIFO now
  case QC_MODE_FIFO:
  default:
    task = container_of(value->head.next, proton_coroutine_task, waiting);
    LL_remove(&task->waiting);
    break;
  }

  if (task->status != QC_STATUS_SUSPEND) {
    PLOG_ERROR("coroutine[%ld] is not suspend, current status is %d", task->cid,
               task->status);
  }

  int rc = proton_coroutine_swap_in(task);
  if (rc != 0) {
    PLOG_ERROR("coroutine[%ld] wake up failed", task->cid);
  } else if (out_task != NULL) {
    *out_task = task;
  }
  return rc;
}

int proton_coroutine_schedule(proton_coroutine_runtime *runtime) {
  MAKESURE_PTR_NOT_NULL(runtime);

  if (RUNTIME_CURRENT_COROUTINE(runtime) != RUNTIME_MAIN_COROUTINE(runtime)) {
    PLOG_ERROR("coroutine_schedule must running on main coroutine");
  }

  if (!LL_isspin(&runtime->releaseables)) {
    int max_release_batch = 500;
    list_link_t *p = runtime->releaseables.next;
    while (p != &runtime->releaseables && max_release_batch-- > 0) {
      proton_coroutine_task *task =
          container_of(p, proton_coroutine_task, link);
      p = LL_remove(p);
      if (task != NULL) {
        // PLOG_DEBUG("found task to release(%p)", task);
        RELEASE_VALUE_MYSELF(task->value);
      }
    }
  }

  if (!LL_isspin(&runtime->runables)) {
    list_link_t *p = runtime->runables.next;
    LL_remove(runtime->runables.next);
    proton_coroutine_task *task =
        container_of(p, proton_coroutine_task, runable);
    if (task == NULL) {
      PLOG_WARN("invalidate task from runable link");
      return -1;
    }

    proton_coroutine_swap_in(task);
  }

  return 0;
}