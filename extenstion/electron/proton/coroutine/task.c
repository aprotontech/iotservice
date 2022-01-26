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

#include "php.h"
#include "ext/standard/info.h"

typedef struct _task_context_wrap_t {
  quark_coroutine_task task;
  quark_coroutine_entry entry;
  zend_execute_data *call;
  zend_function *func;
} task_context_wrap_t;

#define QUARK_COROUTINE_ALIGN_SIZE(size)                                       \
  (((sizeof(task_context_wrap_t) % size) + 1) * size)

static task_context_wrap_t __main_coroutine;

extern quark_coroutine_task *_current_coroutine;

static uint64_t __next_coroutine_id = 0;

zend_vm_stack vm_stack_init(uint32_t size);
zend_execute_data *init_coroutinue_php_stack(quark_coroutine_task *task,
                                             quark_coroutine_entry *entry);

static void _quark_task_runner(task_context_wrap_t *wrap) {
  void run_quark_coroutine_task(quark_coroutine_task * task,

                                zend_execute_data * call, zend_function * func);

  run_quark_coroutine_task(&wrap->task, wrap->call, wrap->func);
}

quark_coroutine_task *quark_coroutine_current() { return _current_coroutine; }

quark_coroutine_task *quark_coroutine_get_main() {
  return &__main_coroutine.task;
}

int _is_real_coroutine(quark_coroutine_task *task) {
  return task != NULL && task != quark_coroutine_get_main();
}

int quark_coroutine_env_init() {
  _current_coroutine = &__main_coroutine.task;
  memset(&__main_coroutine, 0, sizeof(task_context_wrap_t));
  LL_init(&__main_coroutine.task.link);
  __main_coroutine.task.origin = NULL;
  __main_coroutine.task.parent = NULL;
  __main_coroutine.task.vars = hashmap_new();
  __main_coroutine.task.status = QC_STATUS_RUNNING;

  return 0;
}

quark_coroutine_task *quark_coroutine_create(quark_coroutine_task *current,
                                             quark_coroutine_entry *entry,
                                             int c_stack_size,
                                             int php_stack_size) {
  quark_coroutine_task *task = NULL;
  char *ptr = NULL;
  int aligned_size = QUARK_COROUTINE_ALIGN_SIZE(16);

  ptr = (char *)qmalloc(aligned_size + DEFAULT_C_STACK_SIZE);

  task = &((task_context_wrap_t *)ptr)->task;

  getcontext(&task->context);
  task->context.uc_stack.ss_sp = ptr + aligned_size;
  task->context.uc_stack.ss_size = DEFAULT_C_STACK_SIZE;
  task->context.uc_stack.ss_flags = 0;
  task->context.uc_link = &__main_coroutine.task.context;

  task->cid = ++__next_coroutine_id;
  task->parent = current;
  task->origin = NULL;
  task->status = QC_STATUS_FREE;
  task->page = vm_stack_init(DEFAULT_PHP_STACK_PAGE_SIZE);
  LL_init(&task->link);
  LL_init(&task->waiting);
  task->ref_count = 0;
  task->vars = NULL;
  ZVAL_UNDEF(&task->value.myself);
  if (_is_real_coroutine(task->parent)) {
    Z_TRY_ADDREF(task->parent->value.myself);
  }

  ((task_context_wrap_t *)ptr)->entry = *entry;
  ((task_context_wrap_t *)ptr)->call = init_coroutinue_php_stack(task, entry);
  ((task_context_wrap_t *)ptr)->func = entry->fci_cache->function_handler;

  makecontext(&(task->context), (void (*)(void))(_quark_task_runner), 1,
              (task_context_wrap_t *)ptr);

  return task;
}

int _free_var_item(any_t item, const char *key, any_t data) {
  quark_coroutine_var_t *var = (quark_coroutine_var_t *)item;
  RELEASE_MYSELF(var->val);
  qfree(var);
  return MAP_OK;
}

void quark_coroutine_destory(quark_coroutine_task *task) {
  if (task != NULL) {
    QUARK_LOGGER("[COROUTINE] task(%lu) destory", task->cid);
    if (task->status == QC_STATUS_STOPED) {
      LL_remove(&task->link);
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
      QUARK_LOGGER("[COROUTINE] [INVALIDATE STATUS]");
    }
  }
}

quark_coroutine_task *save_vm_stack(quark_coroutine_task *task) {
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

zend_execute_data *init_coroutinue_php_stack(quark_coroutine_task *task,
                                             quark_coroutine_entry *entry) {
  zend_execute_data *call = NULL;
  zend_fcall_info_cache fci_cache = *entry->fci_cache;
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
    ZVAL_PTR_DTOR(arg); // release argv, no need now
  }

  call->symbol_table = NULL;
  return call;
}

void run_quark_coroutine_task(quark_coroutine_task *task,
                              zend_execute_data *call, zend_function *func) {

  zval retval;

  EG(current_execute_data) = call;

  save_vm_stack(task);

  if (func->type == ZEND_USER_FUNCTION) {
    ZVAL_UNDEF(&retval);
    EG(current_execute_data) = NULL;
    zend_init_func_execute_data(call, &func->op_array, &retval);
    zend_execute_ex(EG(current_execute_data));
  }

  zval_ptr_dtor(&retval);

  task->status = QC_STATUS_STOPED;

  QUARK_LOGGER("[COROUTINE] [finish] switch(%lu->%lu)", task->cid,
               quark_coroutine_get_main()->cid);
  _current_coroutine = quark_coroutine_get_main();

  if (_is_real_coroutine(task->parent)) {
    RELEASE_MYSELF(task->parent->value.myself);
    task->parent = NULL;
  }

  if (_is_real_coroutine(task->origin)) {
    // [task] no need [origin] now, so un-ref it
    RELEASE_MYSELF(task->origin->value.myself);
    task->origin = NULL;
  }

  QUARK_LOGGER("[COROUTINE] id=%lu, myself refcount=%d", task->cid,
               Z_REFCOUNT(task->value.myself));
  RELEASE_MYSELF(task->value.myself);
  ZVAL_UNDEF(&task->value.myself);
}
