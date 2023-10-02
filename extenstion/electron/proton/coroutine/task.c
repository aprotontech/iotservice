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
  zend_execute_data *call;
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
zend_function *init_coroutinue_php_stack(task_context_wrap_t *wrap);
static void _quark_task_runner(task_context_wrap_t *wrap);

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
  task->ehandler = NULL;

  LL_init(&task->runable);
  LL_init(&task->waiting);
  LL_init(&task->link);
  LL_init(&task->notify);
  task->vars = NULL;
  ZVAL_UNDEF(&task->value.myself);
  ZVAL_UNDEF(&task->retval);
  task->value.type = &__proton_coroutine_type;
  if (IS_REAL_COROUTINE(task->parent)) {
    Z_TRY_ADDREF(task->parent->value.myself);
  }

  // init php-stack
  task->vm_stack_page_size = php_stack_size;
  task->page = vm_stack_init(php_stack_size);
  ptr->entry = *entry;
  ptr->func = init_coroutinue_php_stack(ptr);

  makecontext(&(task->context), (void (*)(void))(_quark_task_runner), 1, ptr);

  return task;
}

extern int _free_var_item(any_t item, const char *key, any_t data);
int proton_coroutine_destory(proton_private_value_t *value) {
  proton_coroutine_task *task = (proton_coroutine_task *)value;
  MAKESURE_PTR_NOT_NULL(task);

  PLOG_INFO("[COROUTINE] task(%lu) destory", task->cid);
  if (task->status == QC_STATUS_STOPED) {
    zval_ptr_dtor(&task->retval);

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

    list_link_t *q = task->notify.next;
    while (q != &task->notify) {
      q = LL_remove(q);
      // must not reach here
    }
  } else {
    PLOG_ERROR("[COROUTINE] [INVALIDATE STATUS] status=%d", task->status);
  }

  return 0;
}

void save_vm_stack(proton_coroutine_task *task) {
  task->vm_stack = EG(vm_stack);
  task->vm_stack_top = EG(vm_stack_top);
  task->vm_stack_end = EG(vm_stack_end);
  task->vm_stack_page_size = EG(vm_stack_page_size);
  task->execute_data = EG(current_execute_data);
  task->exception = EG(exception);
  task->exception_class = EG(exception_class);
  task->error_handling = EG(error_handling);
  task->bailout = EG(bailout);
}

void restore_vm_stack(proton_coroutine_task *task) {
  EG(vm_stack) = task->vm_stack;
  EG(vm_stack_top) = task->vm_stack_top;
  EG(vm_stack_end) = task->vm_stack_end;
  EG(vm_stack_page_size) = task->vm_stack_page_size;
  EG(current_execute_data) = task->execute_data;
  EG(exception) = task->exception;
  EG(exception_class) = task->exception_class;
  EG(error_handling) = task->error_handling;
  EG(bailout) = task->bailout;
}

zend_vm_stack vm_stack_init(uint32_t size) {
  zend_vm_stack page = (zend_vm_stack)malloc(size);

  page->top = ZEND_VM_STACK_ELEMENTS(page) + 1;
  page->end = (zval *)((char *)page + size);
  page->prev = NULL;

  return page;
}

zend_function *init_coroutinue_php_stack(task_context_wrap_t *wrap) {
  proton_coroutine_task *task = &wrap->task;
  proton_coroutine_entry *entry = &wrap->entry;
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
  task->exception = NULL;
  task->exception_class = NULL;
  task->error_handling = EH_NORMAL;
  task->bailout = NULL;

  save_vm_stack(task->parent);
  restore_vm_stack(task);

  uint32_t call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED;
  if (fci_cache.object != NULL) {
    PLOG_DEBUG("[COROUTINE] task(%lu) has this", task->cid);
    call_info |= ZEND_CALL_HAS_THIS;
  }

  void *obj_or_scope = fci_cache.object;
  if (obj_or_scope == NULL) {
    obj_or_scope = fci_cache.called_scope;
  }

  call =
      zend_vm_stack_push_call_frame(call_info, func, entry->argc, obj_or_scope);

  call->symbol_table = NULL;
  EG(current_execute_data) = call;

  for (int i = 0; i < entry->argc; ++i) {
    zval *arg = &entry->argv[i];
    zval *param = ZEND_CALL_ARG(call, i + 1);
    ZVAL_COPY(param, arg);
  }
  save_vm_stack(task);
  restore_vm_stack(task->parent);

  wrap->call = call;

  return func;
}

void print_coroutine_exception_stack();
void run_proton_coroutine_task(proton_coroutine_task *task, zend_function *func,
                               proton_coroutine_entry *entry) {

  proton_coroutine_runtime *runtime = task->runtime;

  zend_execute_data *call = EG(current_execute_data);

  zval retval;
  ZVAL_UNDEF(&retval);

  PLOG_INFO("[COROUTINUE] task(%lu) start to run func. type=%d", task->cid,
            func->type);
  if (func->type == ZEND_USER_FUNCTION) {
    EG(current_execute_data) = NULL;
    zend_init_func_execute_data(call, &func->op_array, &retval);
    zend_execute_ex(EG(current_execute_data));
  } else if (func->type == ZEND_INTERNAL_FUNCTION) {
    EG(current_execute_data) = NULL;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
    char *is_callable_error = NULL;
    if (zend_fcall_info_init(&entry->func, 0, &fci, &fci_cache, NULL,
                             &is_callable_error) == SUCCESS) {
      zval params;
      ZVAL_NEW_ARR(&params);
      zend_hash_init(Z_ARRVAL(params), entry->argc, NULL, ZVAL_PTR_DTOR, 0);
      for (int i = 0; i < entry->argc; ++i) {
        zval *arg = ZEND_CALL_ARG(call, i + 1);
        zend_hash_index_add(Z_ARRVAL(params), i, arg);
      }
      zend_fcall_info_call(&fci, &fci_cache, &retval, &params);
      ZVAL_PTR_DTOR(&params);
    } else {
      PLOG_WARN("[COROUTINUE] task(%lu) get func info failed, skip it",
                task->cid);
    }
  }

  if (EG(exception) != NULL) {                          // exception is error
    if (Z_TYPE(task->runtime->last_error) != IS_NULL) { // clean-org-error
      ZVAL_PTR_DTOR(&task->runtime->last_error);
    }
    ZVAL_OBJ(&task->runtime->last_error, EG(exception));
    Z_TRY_ADDREF(task->runtime->last_error);

    print_coroutine_exception_stack();

    if (task->ehandler != NULL) {
      task->ehandler(task); // fire the exception
    }
  }

  ZVAL_COPY(&task->retval, &retval);
  ZVAL_PTR_DTOR(&retval); // dec ref

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

  // don't call RELEASE_VALUE_MYSELF(task->value.myself) here
  // because may use [task->context.uc_link] after this function stoped
  LL_remove(&task->link);
  LL_insert(&task->link, task->runtime->releaseables.prev);

  // save current exception to task
  save_vm_stack(task);
}

static void _quark_task_runner(task_context_wrap_t *wrap) {

  restore_vm_stack(&wrap->task);
  run_proton_coroutine_task(&wrap->task, wrap->func, &wrap->entry);

  // restore to main php-stack
  restore_vm_stack(RUNTIME_MAIN_COROUTINE(wrap->task.runtime));

  proton_coroutine_notify_reschedule(wrap->task.runtime);

  // release call
  for (int i = 0; i < wrap->entry.argc; ++i) {
    zval *param = ZEND_CALL_ARG(wrap->call, i + 1);
    //  ZVAL_PTR_DTOR(param); // release argv, no need now
  }

  ZVAL_PTR_DTOR(&wrap->entry.func);
}

void print_stack_frame(zval *exception) {
  zval *file = NULL, *line = NULL;
  if (Z_TYPE_P(exception) == IS_OBJECT) {
    file = zend_read_property(Z_OBJ_P(exception)->ce, exception,
                              ZEND_STRL("file"), 0 TSRMLS_CC, NULL);
    line = zend_read_property(Z_OBJ_P(exception)->ce, exception,
                              ZEND_STRL("line"), 0 TSRMLS_CC, NULL);
  } else if (Z_TYPE_P(exception) == IS_ARRAY) {
    file = zend_hash_str_find(Z_ARRVAL_P(exception), "file", strlen("file"));
    line = zend_hash_str_find(Z_ARRVAL_P(exception), "line", strlen("line"));
  }

  if (file != NULL && line != NULL) {
    PLOG_ERROR("[STACK-TRACE] %s:%d", Z_STRVAL_P(file), Z_LVAL_P(line));
  }
}

void print_coroutine_exception_stack() {
  const char *class_name = "";
  zend_class_entry *ece = EG(exception_class);
  if (ece == NULL) {
    ece = EG(exception)->ce;
  }
  if (ece != NULL) {
    class_name = ZSTR_VAL(ece->name);
  }

  zval exception;
  ZVAL_OBJ(&exception, EG(exception));
  zval *trace = zend_read_property(ece, &exception, ZEND_STRL("trace"),
                                   0 TSRMLS_CC, NULL);
  zval *msg = zend_read_property(ece, &exception, ZEND_STRL("message"),
                                 0 TSRMLS_CC, NULL);

  PLOG_ERROR("exception class(%s), message(%s)", class_name, Z_STRVAL_P(msg));

  print_stack_frame(&exception);

  PLOG_DEBUG("trace=%p, type=%d", trace, Z_TYPE_P(trace));
  if (trace == NULL || Z_TYPE_P(trace) != IS_ARRAY) { // try by debug
    if (Z_OBJ(exception)->handlers != NULL &&
        Z_OBJ(exception)->handlers->get_properties != NULL) {
      HashTable *ht =
          zend_get_properties_for(&exception, ZEND_PROP_PURPOSE_DEBUG);
      if (ht != NULL) {
        zend_string *string_key;
        zend_ulong num_key;
        zval *item = NULL;
        ZEND_HASH_FOREACH_KEY_VAL_IND(ht, num_key, string_key, item) {
          if (string_key != NULL) {
            const char *prop_name, *class_name;
            size_t prop_len;
            int mangled = zend_unmangle_property_name_ex(
                string_key, &class_name, &prop_name, &prop_len);
            if (prop_len == strlen("trace") &&
                strncasecmp(prop_name, "trace", strlen("trace")) == 0) {
              trace = item;
              break;
            }
          }
        }
        ZEND_HASH_FOREACH_END();

        PLOG_DEBUG("trace=%p, type=%d", trace,
                   trace != NULL ? Z_TYPE_P(trace) : 0);
      }
    }
  }

  if (trace != NULL && Z_TYPE_P(trace) == IS_ARRAY) {
    zval *val;
    HashTable *ht = Z_ARRVAL_P(trace);
    ZEND_HASH_FOREACH_VAL(ht, val) { print_stack_frame(val); }
    ZEND_HASH_FOREACH_END();
  }
}
