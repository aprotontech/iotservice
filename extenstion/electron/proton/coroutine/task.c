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
  zend_function *func;
} task_context_wrap_t;

int proton_coroutine_destory(proton_private_value_t *task);

PROTON_TYPE_WHOAMI_DEFINE(_coroutine_get_type, "coroutine")

static proton_value_type_t __proton_coroutine_type = {
    .construct = NULL,
    .destruct = proton_coroutine_destory,
    .whoami = _coroutine_get_type};

#define proton_coroutine_ALIGN_SIZE(size)                                      \
  (((sizeof(task_context_wrap_t) % size) + 1) * size)

static uint64_t __next_coroutine_id = 0;

zend_vm_stack vm_stack_init(uint32_t size);
zend_function *init_coroutinue_php_stack(proton_coroutine_task *task,
                                         proton_coroutine_entry *entry);

static void _quark_task_runner(task_context_wrap_t *wrap) {
  void run_proton_coroutine_task(proton_coroutine_task * task,
                                 zend_function * func);

  run_proton_coroutine_task(&wrap->task, wrap->func);
}

proton_coroutine_task *_proton_coroutine_create(proton_coroutine_task *current,
                                                proton_coroutine_entry *entry,
                                                int c_stack_size,
                                                int php_stack_size) {

  task_context_wrap_t *ptr =
      (task_context_wrap_t *)qmalloc(sizeof(task_context_wrap_t));

  proton_coroutine_task *task = &ptr->task;

  memset(ptr, 0, sizeof(task_context_wrap_t));

  task->c_stack = malloc(c_stack_size);

  getcontext(&task->context);
  task->context.uc_stack.ss_sp = task->c_stack;
  task->context.uc_stack.ss_size = c_stack_size;
  task->context.uc_stack.ss_flags = 0;
  task->context.uc_link = &(RUNTIME_MAIN_COROUTINE(current->runtime)->context);

  task->cid = ++__next_coroutine_id;
  task->parent = current;
  task->origin = NULL;
  task->status = QC_STATUS_FREE;

  LL_init(&task->runable);
  LL_init(&task->waiting);
  LL_init(&task->link);
  task->vars = NULL;
  ZVAL_UNDEF(&task->value.myself);
  task->value.type = &__proton_coroutine_type;
  if (IS_REAL_COROUTINE(task->parent)) {
    Z_TRY_ADDREF(task->parent->value.myself);
  }

  // init php-stack
  task->vm_stack_page_size = php_stack_size;
  task->page = vm_stack_init(php_stack_size);
  ptr->entry = *entry;
  ptr->func = init_coroutinue_php_stack(task, entry);

  makecontext(&(task->context), (void (*)(void))(_quark_task_runner), 1, ptr);

  return task;
}

extern int _free_var_item(any_t item, const char *key, any_t data);
int proton_coroutine_destory(proton_private_value_t *value) {
  proton_coroutine_task *task = (proton_coroutine_task *)value;
  MAKESURE_PTR_NOT_NULL(task);

  PLOG_INFO("[COROUTINE] task(%lu) destory", task->cid);
  if (task->status == QC_STATUS_STOPED) {
    LL_remove(&task->runable);
    LL_remove(&task->waiting);
    LL_remove(&task->link);
    if (task->vars != NULL) { // free all vars
      hashmap_iterate(task->vars, _free_var_item, NULL);
      hashmap_free(task->vars);
      task->vars = NULL;
    }
    if (task->page != NULL) {
      free(task->page);
      task->page = NULL;
    }

    if (task->c_stack != NULL) {
      free(task->c_stack);
      task->c_stack = NULL;
    }
  } else {
    PLOG_ERROR("[COROUTINE] [INVALIDATE STATUS] status=%d", task->status);
  }

  return -1;

  return 0;
}

void save_vm_stack(proton_coroutine_task *task) {
  task->vm_stack = EG(vm_stack);
  task->vm_stack_top = EG(vm_stack_top);
  task->vm_stack_end = EG(vm_stack_end);
  task->vm_stack_page_size = EG(vm_stack_page_size);
  task->execute_data = EG(current_execute_data);
}

void restore_vm_stack(proton_coroutine_task *task) {
  EG(vm_stack) = task->vm_stack;
  EG(vm_stack_top) = task->vm_stack_top;
  EG(vm_stack_end) = task->vm_stack_end;
  EG(vm_stack_page_size) = task->vm_stack_page_size;
  EG(current_execute_data) = task->execute_data;
}

zend_vm_stack vm_stack_init(uint32_t size) {
  zend_vm_stack page = (zend_vm_stack)malloc(size);

  page->top = ZEND_VM_STACK_ELEMENTS(page) + 1;
  page->end = (zval *)((char *)page + size);
  page->prev = NULL;

  return page;
}

zend_function *init_coroutinue_php_stack(proton_coroutine_task *task,
                                         proton_coroutine_entry *entry) {
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
  task->vm_stack = page;
  task->vm_stack_top = page->top;
  task->vm_stack_end = page->end;
  task->execute_data = NULL;

  save_vm_stack(task->parent);
  restore_vm_stack(task);

  call = zend_vm_stack_push_call_frame(
      ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED, func, entry->argc,
      fci_cache.object != NULL ? (void *)fci_cache.object
                               : (void *)fci_cache.called_scope);

  call->symbol_table = NULL;
  EG(current_execute_data) = call;

  for (int i = 0; i < entry->argc; ++i) {
    zval *param;
    zval *arg = &entry->argv[i];
    param = ZEND_CALL_ARG(call, i + 1);
    ZVAL_COPY(param, arg);
    // ZVAL_PTR_DTOR(arg); // release argv, no need now
  }
  save_vm_stack(task);
  restore_vm_stack(task->parent);

  return func;
}

void run_proton_coroutine_task(proton_coroutine_task *task,
                               zend_function *func) {

  proton_coroutine_runtime *runtime = task->runtime;

  zval retval;
  ZVAL_UNDEF(&retval);

  zend_execute_data *call = EG(current_execute_data);

  if (func->type == ZEND_USER_FUNCTION) {
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

  PLOG_INFO("[COROUTINE] [finish] switch(%lu->%lu), refcount=%d", task->cid,
            RUNTIME_MAIN_COROUTINE(runtime)->cid,
            Z_REFCOUNT(task->value.myself) - 1);

  LL_remove(&task->link);
  LL_insert(&task->link, task->runtime->releaseables.prev);

  // restore to main php-stack
  restore_vm_stack(RUNTIME_MAIN_COROUTINE(runtime));
}
