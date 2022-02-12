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

#ifndef _proton_coroutine_TASK_H_
#define _proton_coroutine_TASK_H_

#include <stdint.h>
#include <ucontext.h>
#include <unistd.h>

#include "proton/common/electron.h"

#define DEFAULT_C_STACK_SIZE (1024 * 1024)
#define DEFAULT_PHP_STACK_PAGE_SIZE 8192

typedef enum _proton_coroutine_status {
  QC_STATUS_FREE = 0,
  QC_STATUS_RUNABLE,
  QC_STATUS_RUNNING,
  QC_STATUS_SUSPEND,
  QC_STATUS_STOPED,
} proton_coroutine_status;

typedef struct _proton_coroutine_runtime_t proton_coroutine_runtime;
typedef struct _proton_coroutine_task {
  proton_private_value_t value;

  // PHP stack
  zval *vm_stack_top;
  zval *vm_stack_end;
  zend_vm_stack vm_stack;
  size_t vm_stack_page_size;
  zend_execute_data *execute_data;
  zend_error_handling_t error_handling;
  zend_class_entry *exception_class;
  zend_object *exception;
  JMP_BUF *bailout;

  void *c_stack;
  zend_vm_stack page;

  struct _proton_coroutine_task *parent; // no use now
  struct _proton_coroutine_task *origin;

  proton_coroutine_status status;
  uint64_t cid;

  ucontext_t context;

  map_t vars;

  proton_coroutine_runtime *runtime; // belongs to

  // all coroutinues list (belongs to runtime)
  list_link_t link;

  // if current task is runable, it will be inserted to a list, this link to
  // other coroutinues
  list_link_t runable;

  // if more than one corotinue waiting for a object, this link to other
  // corotinues.
  // a corotinue only can waiting for one object
  list_link_t waiting;
} proton_coroutine_task;

typedef struct _proton_coroutine_entry {
  zval func;
  zval *argv;
  uint32_t argc;
} proton_coroutine_entry;

#endif