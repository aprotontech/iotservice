/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     task.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-03 10:00:37
 *
 */

#ifndef _QUARK_COROUTINE_TASK_H_
#define _QUARK_COROUTINE_TASK_H_

#include <stdint.h>
#include <ucontext.h>
#include <unistd.h>

#include "php.h"
#include "proton/include/electron.h"

#define DEFAULT_C_STACK_SIZE (1024 * 1024)
#define DEFAULT_PHP_STACK_PAGE_SIZE 8192

//#define RELEASE_ZVAL(p) zval_dtor(&(p))

#define RELEASE_ZVAL(p)

typedef enum _quark_coroutine_status {
  QC_STATUS_FREE = 0,
  QC_STATUS_RUNABLE,
  QC_STATUS_RUNNING,
  QC_STATUS_SUSPEND,
  QC_STATUS_STOPED,
} quark_coroutine_status;

typedef enum _quark_coroutine_var_mode {
  QC_VAR_SHARE = 0,
  QC_VAR_READONLY,
  QC_VAR_INHERIT,
} quark_coroutine_var_mode;

typedef struct _quark_coroutine_var_t {
  zval val;
  quark_coroutine_var_mode mode;
  char set_by_me;
  char key[4];
} quark_coroutine_var_t;

typedef struct _quark_coroutine_task {
  proton_private_value_t value;
  zval *vm_stack_top;
  zval *vm_stack_end;
  zend_vm_stack vm_stack;
  size_t vm_stack_page_size;
  zend_execute_data *execute_data;

  zend_vm_stack page;

  struct _quark_coroutine_task *parent; // no use now
  struct _quark_coroutine_task *origin;

  quark_coroutine_status status;
  uint64_t cid;

  int ref_count; // no use now

  zval myself;

  ucontext_t context;

  map_t vars;
  list_link_t link;

  // if more than one corotinue waiting for a object, this link to other
  // corotinues.
  // a corotinue only can waiting for one object
  list_link_t waiting;
} quark_coroutine_task;

typedef struct _quark_coroutine_entry {
  zend_fcall_info_cache *fci_cache;
  zval *argv;
  uint32_t argc;
} quark_coroutine_entry;

int quark_coroutine_env_init();
quark_coroutine_task *quark_coroutine_get_main();

quark_coroutine_task *quark_coroutine_create(quark_coroutine_task *current,
                                             quark_coroutine_entry *entry,
                                             int c_stack_size,
                                             int php_stack_size);

void quark_coroutine_destory(quark_coroutine_task *task);

quark_coroutine_task *quark_coroutine_current();

// swap to coroutine[dest]
int quark_coroutine_swap_in(quark_coroutine_task *dest);

// from coroutine[current] swap out
int quark_coroutine_swap_out(quark_coroutine_task *current,
                             quark_coroutine_status new_status);

int quark_coroutine_get_var(quark_coroutine_task *current, const char *key,
                            zval *val);

int quark_coroutine_set_var(quark_coroutine_task *current, const char *key,
                            zval *val, quark_coroutine_var_mode mode);

#endif