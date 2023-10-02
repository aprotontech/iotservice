/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     wait_group.h
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-11-06 11:31:52
 *  @version  0
 *  @brief
 *
 * *
 */
#ifndef _ELECTRON_WAIT_GROUP_H_
#define _ELECTRON_WAIT_GROUP_H_

#include "task.h"
#include "runtime.h"

typedef struct _proton_wait_group_t proton_wait_group_t;

typedef enum _proton_waitgroup_mode_status {
  WG_WAIT_ONEOF = 1,
  WG_WAIT_ALL = 2
} proton_waitgroup_mode_status;

typedef struct _proton_notify_item_t {
  list_link_t colink;
  list_link_t wglink;

  int is_finished;

  proton_private_value_t *task;
  proton_wait_group_t *group;
} proton_notify_item_t;

typedef struct _proton_wait_group_t {
  proton_private_value_t value;

  proton_coroutine_runtime *runtime;

  list_link_t items;
  int nitem_total;
  int nitem_finished;

  int wait_mode;

  proton_notify_item_t *last_notify_task;

  list_link_t link;
  // if is waiting
  proton_wait_object_t wait;
} proton_wait_group_t;

proton_private_value_t *
proton_wait_group_create(proton_coroutine_runtime *runtime, int mode);

int proton_wait_group_add(proton_private_value_t *wg,
                          proton_private_value_t *task);

int proton_wait_group_waitfor(proton_private_value_t *wg);

int proton_wait_group_notify(proton_private_value_t *task, list_link_t *item);

int proton_wait_group_close(proton_private_value_t *wg);

#endif