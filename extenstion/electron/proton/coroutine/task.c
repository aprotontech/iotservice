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

#include "ext/standard/info.h"
#include "php.h"

typedef struct _task_context_wrap_t {
  quark_coroutine_task task;
  quark_coroutine_entry entry;
} task_context_wrap_t;

#define QUARK_COROUTINE_ALIGN_SIZE(size)                                       \
  (((sizeof(task_context_wrap_t) % size) + 1) * size)

static task_context_wrap_t __main_coroutine;

static quark_coroutine_task *_current_coroutine = NULL;

static uint64_t __next_coroutine_id = 0;

zend_vm_stack vm_stack_init(uint32_t size);

static void _quark_task_runner(task_context_wrap_t *wrap) {
  void run_quark_coroutine_task(quark_coroutine_task * task,
                                quark_coroutine_entry * entry);

  run_quark_coroutine_task(&wrap->task, &wrap->entry);
}

ucontext_t *_get_current_ucontext() {
  assert(_current_coroutine != NULL);

  return &_current_coroutine->context;
}

quark_coroutine_task *quark_coroutine_current() { return _current_coroutine; }

quark_coroutine_task *quark_coroutine_get_main() {
  return &__main_coroutine.task;
}

int quark_coroutine_env_init() {
  _current_coroutine = &__main_coroutine.task;
  memset(&__main_coroutine, 0, sizeof(task_context_wrap_t));
  LL_init(&__main_coroutine.task.link);
  __main_coroutine.task.origin = NULL;
  __main_coroutine.task.parent = NULL;
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
  task->ref_count = 0;
  ZVAL_UNDEF(&task->myself);

  ((task_context_wrap_t *)ptr)->entry = *entry;

  makecontext(&(task->context), (void (*)(void))(_quark_task_runner), 1,
              (task_context_wrap_t *)ptr);

  /*
    // save context to swap_org_ctx, and switch to _quark_task_runner
    quark_coroutine_swap_in(task);

    // .... _quark_task_runner is running now

    // when _quark_task_runner suspended, will switch to here
  */
  return task;
}

void quark_coroutine_destory(quark_coroutine_task *task) {
  if (task != NULL) {
    QUARK_LOGGER("[COROUTINE] task(%lu) destory", task->cid);
    if (task->status == QC_STATUS_STOPED) {
      LL_remove(&task->link);
      if (task->page != NULL) {
        qfree(task->page);
      }

      qfree(task);
    } else {
      QUARK_LOGGER("[COROUTINE] [INVALIDATE STATUS]");
    }
  }
}

int quark_coroutine_swap_in(quark_coroutine_task *dest) {
  if (dest == NULL || dest == _current_coroutine) {
    QUARK_LOGGER("[COROUTINE] swap failed. input coroutinue can't swap in");
    return -1;
  }

  ucontext_t *org_context = _get_current_ucontext();

  dest->status = QC_STATUS_RUNNING;
  dest->origin = _current_coroutine;

  QUARK_LOGGER("[COROUTINE] [resume] switch(%lu->%lu)", _current_coroutine->cid,
               dest->cid);

  _current_coroutine = dest;

  // jump to [dest->context], save to [org_context]
  swapcontext(org_context, &(dest->context));
  return 0;
}

int quark_coroutine_swap_out(quark_coroutine_task *current,
                             quark_coroutine_status new_status) {
  if (current == NULL || current != _current_coroutine) {
    QUARK_LOGGER("[COROUTINE] swap failed. input coroutinue can't swap out");
    return -1;
  } else if (current == quark_coroutine_get_main()) {
    QUARK_LOGGER("[COROUTINE] swap failed. main coroutinue can't swap out");
    return -1;
  }
  if (new_status != QC_STATUS_RUNABLE && new_status != QC_STATUS_SUSPEND) {
    QUARK_LOGGER("[COROUTINE] swap failed. input status must be "
                 "QC_STATUS_RUNABLE or QC_STATUS_SUSPEND.");
    return -1;
  }

  assert(current->origin != NULL);

  current->status = new_status;

  // origin coroutinue is waiting for some object or signal
  // so swap to main coroutine(schedule to a runable coroutinue)
  quark_coroutine_status org_status = current->origin->status;
  if (org_status == QC_STATUS_SUSPEND || org_status == QC_STATUS_STOPED) {
    QUARK_LOGGER(
        "[COROUTINE] origin coroutine is suspend or stoped, so switch to main");
    current->origin = quark_coroutine_get_main();
  }

  QUARK_LOGGER("[COROUTINE] [yield] switch(%lu->%lu)", current->cid,
               current->origin->cid);

  current->origin->status = QC_STATUS_RUNNING;
  _current_coroutine = current->origin; // switch to origin coroutine

  // jump to [_get_current_ucontext()], save to [current->context]
  swapcontext(&(current->context), _get_current_ucontext());
  return 0;
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

void run_quark_coroutine_task(quark_coroutine_task *task,
                              quark_coroutine_entry *entry) {

  zval retval;
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
  }

  call->symbol_table = NULL;

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
}