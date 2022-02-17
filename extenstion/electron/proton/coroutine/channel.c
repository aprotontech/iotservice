/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     channel.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-05 12:45:47
 *
 */

#include "channel.h"

PROTON_TYPE_WHOAMI_DEFINE(_channel_get_type, "channel")

static proton_value_type_t __proton_channel_type = {
    .construct = NULL,
    .destruct = proton_channel_uninit,
    .whoami = _channel_get_type};

typedef struct _proton_channel_value_t {
  zval value;
  list_link_t link;
} proton_channel_value_t;

int proton_channel_init(proton_coroutine_runtime *runtime,
                        proton_channel_t *channel, uint64_t max_size) {
  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(channel);
  if (max_size == 0) {
    PLOG_WARN("channel current not support maxsize=0");
    return -1;
  }
  channel->value.type = &__proton_channel_type;
  channel->runtime = runtime;
  channel->max_size = max_size;
  channel->current_size = 0;
  LL_init(&channel->head);
  PROTON_WAIT_OBJECT_INIT(channel->wq_free);
  PROTON_WAIT_OBJECT_INIT(channel->wq_full);
  return 0;
}

int proton_channel_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_channel_t *channel = (proton_channel_t *)value;

  if (IS_COROUTINE_WAITFOR(channel->wq_free)) {
    proton_coroutine_cancel(channel->runtime, &channel->wq_free, NULL);
  }

  if (IS_COROUTINE_WAITFOR(channel->wq_full)) {
    proton_coroutine_cancel(channel->runtime, &channel->wq_full, NULL);
  }

  list_link_t *p = channel->head.next;
  while (p != &channel->head) {
    proton_channel_value_t *v = container_of(p, proton_channel_value_t, link);
    p = LL_remove(p);
    ZVAL_PTR_DTOR(&v->value);
    qfree(v);
  }

  channel->current_size = 0;

  return 0;
}

int proton_channel_push(proton_private_value_t *value, zval *item) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(item);
  proton_channel_t *channel = (proton_channel_t *)value;
  MAKESURE_ON_COROTINUE(channel->runtime);

  if (channel->current_size >= channel->max_size) {
    if (proton_coroutine_waitfor(channel->runtime, &channel->wq_full, NULL)) {
      return -1;
    }
  }

  proton_channel_value_t *v =
      (proton_channel_value_t *)qmalloc(sizeof(proton_channel_value_t));
  LL_insert(&v->link, channel->head.prev);
  ZVAL_COPY(&v->value, item);
  ++channel->current_size;
  if (IS_COROUTINE_WAITFOR(channel->wq_free)) {
    return proton_coroutine_wakeup(channel->runtime, &channel->wq_free, NULL);
  }

  return 0;
}

int proton_channel_pop(proton_private_value_t *value, zval *item) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(item);
  proton_channel_t *channel = (proton_channel_t *)value;

  MAKESURE_ON_COROTINUE(channel->runtime);

  if (channel->current_size == 0) {
    if (proton_coroutine_waitfor(channel->runtime, &channel->wq_free, NULL)) {
      return -1;
    }
  }

  if (LL_isspin(&channel->head)) {
    PLOG_ERROR("channel got error, current size is 0");
    return -1;
  }

  proton_channel_value_t *v =
      container_of(channel->head.next, proton_channel_value_t, link);
  LL_remove(&v->link);
  ZVAL_COPY(item, &v->value);
  Z_TRY_DELREF(v->value);
  qfree(v);
  --channel->current_size;

  if (IS_COROUTINE_WAITFOR(channel->wq_full)) {
    return proton_coroutine_wakeup(channel->runtime, &channel->wq_full, NULL);
  }

  return 0;
}

int proton_channel_close(proton_private_value_t *value) { return 0; }