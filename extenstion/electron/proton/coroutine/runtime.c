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

void _runtime_throw_exception(proton_coroutine_task *task);

proton_coroutine_runtime *
proton_runtime_init(proton_coroutine_runtime *runtime) {
  if (runtime != NULL) {

    memset(runtime, 0, sizeof(proton_coroutine_runtime));

    LL_init(&runtime->clist);
    LL_init(&runtime->runables);
    LL_init(&runtime->releaseables);

    ZVAL_NULL(&runtime->error_handler);

    // init main coroutine
    LL_init(&runtime->main.link);
    LL_init(&runtime->main.runable);
    LL_init(&runtime->main.waiting);

    runtime->main.cid = 0;
    runtime->main.runtime = runtime;
    runtime->main.status = QC_STATUS_RUNNING;

    save_vm_stack(&runtime->main);

    runtime->current = &runtime->main;
  }

  return runtime;
}

int proton_runtime_uninit(proton_coroutine_runtime *runtime) {
  MAKESURE_PTR_NOT_NULL(runtime);
  // ZVAL_PTR_DTOR(&runtime->error_handler);
  runtime->current = NULL;

  return 0;
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
    task->ehandler = NULL;

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

  proton_coroutine_notify_reschedule(runtime);

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

    if (value->is_canceled) { // waiting queue is caceled
      return RC_ERROR_COROUTINUE_CANCELED;
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

  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(runtime);
  if (current != NULL && IS_REAL_COROUTINE(current) && current != task) {
    // append dest task to runable list
    task->status = QC_STATUS_RUNABLE;
    LL_remove(&task->runable);
    LL_insert(&task->runable, runtime->runables.prev);
    return proton_coroutine_notify_reschedule(runtime);
  }

  int rc = proton_coroutine_swap_in(task);
  if (rc != 0) {
    PLOG_ERROR("coroutine[%ld] wake up failed", task->cid);
  } else if (out_task != NULL) {
    *out_task = task;
  }
  return rc;
}

int proton_coroutine_cancel(proton_coroutine_runtime *runtime,
                            proton_wait_object_t *value,
                            proton_coroutine_task **task) {
  MAKESURE_PTR_NOT_NULL(value);
  value->is_canceled = 1;

  return proton_coroutine_wakeup(runtime, value, task);
}

void _runtime_throw_exception(proton_coroutine_task *task) {
  zval retval;
  zval params[2];

  PLOG_DEBUG("_runtime_throw_exception[%ld]", task->cid);

  if (!zend_is_callable(&task->runtime->error_handler,
                        IS_CALLABLE_CHECK_NO_ACCESS, NULL)) {
    PLOG_WARN("coroutine[%ld] exit with error, but callback is null",
              task->cid);
    return;
  }

  ZVAL_NULL(&retval);
  ZVAL_COPY(&params[0], &task->value.myself);
  ZVAL_OBJ(&params[1], task->exception);
  if (call_user_function_ex(EG(function_table), NULL,
                            &task->runtime->error_handler, &retval, 2, params,
                            0, NULL TSRMLS_CC) != SUCCESS) {
    PLOG_ERROR("coroutine[%ld] exit with error, but callback failed",
               task->cid);
  }

  Z_TRY_DELREF(params[0]);
  zval_ptr_dtor(&retval);
}

int proton_runtime_cleanup_temp(proton_coroutine_runtime *runtime,
                                int max_release_batch) {
  MAKESURE_PTR_NOT_NULL(runtime);

  if (RUNTIME_CURRENT_COROUTINE(runtime) != RUNTIME_MAIN_COROUTINE(runtime)) {
    PLOG_ERROR("proton_runtime_cleanup_temp must running on main coroutine");
  }

  if (!LL_isspin(&runtime->releaseables)) {
    list_link_t *p = runtime->releaseables.next;
    while (p != &runtime->releaseables && max_release_batch-- > 0) {
      proton_coroutine_task *task =
          container_of(p, proton_coroutine_task, link);
      p = LL_remove(p);
      if (task != NULL) {
        if (task->exception != NULL) { // task exit with exception
          _runtime_throw_exception(task);
        }

        // PLOG_DEBUG("found task to release(%p)", task);
        RELEASE_VALUE_MYSELF(task->value);
      }
    }
  }

  return 0;
}

int proton_coroutine_schedule(proton_coroutine_runtime *runtime) {
  MAKESURE_PTR_NOT_NULL(runtime);

  if (RUNTIME_CURRENT_COROUTINE(runtime) != RUNTIME_MAIN_COROUTINE(runtime)) {
    PLOG_ERROR("coroutine_schedule must running on main coroutine");
  }

  proton_runtime_cleanup_temp(runtime, 500);

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