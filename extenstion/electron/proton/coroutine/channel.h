/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     channel.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-05 12:45:51
 *
 */

#ifndef _PROTON_COROUTINE_CHANNEL_H_
#define _PROTON_COROUTINE_CHANNEL_H_

#include "proton/common/electron.h"
#include "runtime.h"

typedef struct _proton_channel_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  uint64_t max_size;
  uint64_t current_size;

  int closed;

  proton_wait_object_t wq_full;
  proton_wait_object_t wq_free;

  list_link_t head;
} proton_channel_t;

int proton_channel_init(proton_coroutine_runtime *runtime,
                        proton_channel_t *channel, uint64_t max_size);

int proton_channel_uninit(proton_private_value_t *channel);

int proton_channel_push(proton_private_value_t *channel, zval *item);

int proton_channel_pop(proton_private_value_t *channel, zval *item);

int proton_channel_try_push(proton_private_value_t *channel, zval *item);

int proton_channel_is_full(proton_private_value_t *channel);

int proton_channel_close(proton_private_value_t *channel);

#endif