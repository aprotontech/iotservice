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
#include "proton/include/clist.h"
#include "proton/include/electron.h"

#define DEFAULT_C_STACK_SIZE (1024 * 1024)
#define DEFAULT_PHP_STACK_PAGE_SIZE 8192

typedef enum _quark_coroutine_status {
  QC_STATUS_FREE = 0,
  QC_STATUS_RUNABLE,
  QC_STATUS_RUNNING,
  QC_STATUS_SUSPEND,
  QC_STATUS_STOPED,
} quark_coroutine_status;

typedef struct _quark_coroutine_task {
  zval *vm_stack_top;
  zval *vm_stack_end;
  zend_vm_stack vm_stack;
  size_t vm_stack_page_size;
  zend_execute_data *execute_data;

  zend_vm_stack page;

  struct _quark_coroutine_task *parent;
  struct _quark_coroutine_task *origin;

  quark_coroutine_status status;
  uint64_t cid;

  int ref_count;

  zval myself;

  ucontext_t context;

  list_link_t link;
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

#endif