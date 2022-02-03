/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     task.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-03 10:08:52
 *
 */

#include "task.h"
#include "variable.h"
#include "runtime.h"

typedef struct _task_context_wrap_t {
  proton_coroutine_task task;
  proton_coroutine_entry entry;
  zend_execute_data *call;
  zend_function *func;
} task_context_wrap_t;

#define proton_coroutine_ALIGN_SIZE(size)                                      \
  (((sizeof(task_context_wrap_t) % size) + 1) * size)

static uint64_t __next_coroutine_id = 0;

zend_vm_stack vm_stack_init(uint32_t size);
zend_execute_data *init_coroutinue_php_stack(proton_coroutine_task *task,
                                             proton_coroutine_entry *entry,
                                             zend_function **entry_func);

static void _quark_task_runner(task_context_wrap_t *wrap) {
  void run_proton_coroutine_task(proton_coroutine_task * task,
                                 zend_execute_data * call,
                                 zend_function * func);

  run_proton_coroutine_task(&wrap->task, wrap->call, wrap->func);
}

proton_coroutine_task *_proton_coroutine_create(proton_coroutine_task *current,
                                                proton_coroutine_entry *entry,
                                                int c_stack_size,
                                                int php_stack_size) {
  proton_coroutine_task *task = NULL;
  char *ptr = NULL;
  int aligned_size = proton_coroutine_ALIGN_SIZE(16);

  ptr = (char *)qmalloc(aligned_size + c_stack_size);
  memset(ptr, 0, aligned_size + c_stack_size);

  task = &((task_context_wrap_t *)ptr)->task;

  getcontext(&task->context);
  task->context.uc_stack.ss_sp = ptr + aligned_size;
  task->context.uc_stack.ss_size = c_stack_size;
  task->context.uc_stack.ss_flags = 0;
  task->context.uc_link = &(RUNTIME_MAIN_COROUTINE(current->runtime)->context);

  task->cid = ++__next_coroutine_id;
  task->parent = current;
  task->origin = NULL;
  task->status = QC_STATUS_FREE;
  task->page = vm_stack_init(php_stack_size);
  LL_init(&task->runable);
  LL_init(&task->waiting);
  task->vars = NULL;
  ZVAL_UNDEF(&task->value.myself);
  if (IS_REAL_COROUTINE(task->parent)) {
    Z_TRY_ADDREF(task->parent->value.myself);
  }

  zend_function *entry_func = NULL;
  ((task_context_wrap_t *)ptr)->entry = *entry;
  ((task_context_wrap_t *)ptr)->call =
      init_coroutinue_php_stack(task, entry, &entry_func);
  ((task_context_wrap_t *)ptr)->func = entry_func;

  makecontext(&(task->context), (void (*)(void))(_quark_task_runner), 1,
              (task_context_wrap_t *)ptr);

  return task;
}

extern int _free_var_item(any_t item, const char *key, any_t data);
void proton_coroutine_destory(proton_coroutine_task *task) {
  if (task != NULL) {
    PLOG_INFO("[COROUTINE] task(%lu) destory", task->cid);
    if (task->status == QC_STATUS_STOPED) {
      LL_remove(&task->runable);
      LL_remove(&task->waiting);
      if (task->vars != NULL) { // free all vars
        hashmap_iterate(task->vars, _free_var_item, NULL);
        hashmap_free(task->vars);
        task->vars = NULL;
      }
      if (task->page != NULL) {
        qfree(task->page);
      }

      qfree(task);
    } else {
      PLOG_ERROR("[COROUTINE] [INVALIDATE STATUS]");
    }
  }
}

proton_coroutine_task *save_vm_stack(proton_coroutine_task *task) {
  task->vm_stack_top = EG(vm_stack_top);
  task->vm_stack_end = EG(vm_stack_end);
  task->vm_stack = EG(vm_stack);
  task->vm_stack_page_size = EG(vm_stack_page_size);
  task->execute_data = EG(current_execute_data);
  return task;
}

zend_vm_stack vm_stack_init(uint32_t size) {
  zend_vm_stack page = (zend_vm_stack)qmalloc(size);

  page->top = ZEND_VM_STACK_ELEMENTS(page);
  page->end = (zval *)((char *)page + size);
  page->prev = NULL;

  EG(vm_stack) = page;
  EG(vm_stack)->top++;
  EG(vm_stack_top) = EG(vm_stack)->top;
  EG(vm_stack_end) = EG(vm_stack)->end;
  EG(vm_stack_page_size) = size;
  return page;
}

zend_execute_data *init_coroutinue_php_stack(proton_coroutine_task *task,
                                             proton_coroutine_entry *entry,
                                             zend_function **entry_func) {
  zend_execute_data *call = NULL;

  zend_fcall_info fci = empty_fcall_info;
  zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
  char *is_callable_error = NULL;
  if (zend_fcall_info_init(&entry->func, 0, &fci, &fci_cache, NULL,
                           &is_callable_error) != SUCCESS) {
    PLOG_ERROR("get callback function info failed with(%s)", is_callable_error);
    return NULL;
  }

  zend_function *func = fci_cache.function_handler;

  // create a new php stack
  zend_vm_stack page = task->page;

  call = zend_vm_stack_push_call_frame(
      ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED, func, entry->argc,
      fci_cache.object != NULL ? (void *)fci_cache.object
                               : (void *)fci_cache.called_scope);

  for (int i = 0; i < entry->argc; ++i) {
    zval *param;
    zval *arg = &entry->argv[i];
    param = ZEND_CALL_ARG(call, i + 1);
    ZVAL_COPY(param, arg);
    // ZVAL_PTR_DTOR(arg); // release argv, no need now
  }

  call->symbol_table = NULL;
  *entry_func = func;
  return call;
}

void run_proton_coroutine_task(proton_coroutine_task *task,
                               zend_execute_data *call, zend_function *func) {

  zval retval;
  proton_coroutine_runtime *runtime = task->runtime;

  EG(current_execute_data) = call;

  save_vm_stack(task);

  if (func->type == ZEND_USER_FUNCTION) {
    ZVAL_UNDEF(&retval);
    EG(current_execute_data) = NULL;
    zend_init_func_execute_data(call, &func->op_array, &retval);
    zend_execute_ex(EG(current_execute_data));
  }

  zval_ptr_dtor(&retval);

  if (IS_REAL_COROUTINE(task->parent)) {
    RELEASE_VALUE_MYSELF(task->parent->value);
    task->parent = NULL;
  }

  if (IS_REAL_COROUTINE(task->origin)) {
    // [task] no need [origin] now, so un-ref it
    RELEASE_VALUE_MYSELF(task->origin->value);
    task->origin = NULL;
  }

  task->status = QC_STATUS_STOPED;
  RUNTIME_CURRENT_COROUTINE(runtime) = RUNTIME_MAIN_COROUTINE(runtime);

  PLOG_INFO("[COROUTINE] id=%lu, after finished, myself refcount=%d", task->cid,
            Z_REFCOUNT(task->value.myself) - 1);
  PLOG_INFO("[COROUTINE] [finish] switch(%lu->%lu)", task->cid,
            RUNTIME_MAIN_COROUTINE(runtime)->cid);
  // RELEASE_VALUE_MYSELF(task->value);
  // ZVAL_UNDEF(&task->value.myself);
}
