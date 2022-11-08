/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     wait_group.c
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-11-06 10:01:22
 *  @version  0
 *  @brief
 *
 * *
 */

#include "wait_group.h"

PROTON_TYPE_WHOAMI_DEFINE(_wg_get_type, "waitgroup")

static proton_value_type_t __proton_wg_type = {.construct = NULL,
                                               .destruct =
                                                   proton_wait_group_close,
                                               .whoami = _wg_get_type};

static proton_notify_item_t *find_group_item(proton_wait_group_t *wg,
                                             proton_private_value_t *task);

proton_private_value_t *
proton_wait_group_create(proton_coroutine_runtime *runtime, int mode) {
  if (mode != WG_WAIT_ONEOF && mode != WG_WAIT_ALL) {
    PLOG_WARN("input waitgroup mode(%d) is invalidate", mode);
    return NULL;
  }

  proton_wait_group_t *wg =
      (proton_wait_group_t *)qmalloc(sizeof(proton_wait_group_t));
  memset(wg, 0, sizeof(proton_wait_group_t));

  wg->value.type = &__proton_wg_type;
  PROTON_WAIT_OBJECT_INIT(wg->wait);

  wg->runtime = runtime;
  wg->wait_mode = mode;

  wg->nitem_total = 0;
  wg->nitem_finished = 0;
  LL_init(&wg->items);

  wg->last_notify_task = NULL;

  return &wg->value;
}

int proton_wait_group_add(proton_private_value_t *value,
                          proton_private_value_t *task) {
  proton_wait_group_t *wg = (proton_wait_group_t *)value;
  proton_coroutine_task *ct = (proton_coroutine_task *)task;

  MAKESURE_PTR_NOT_NULL(wg);
  MAKESURE_PTR_NOT_NULL(ct);

  if (IS_COROUTINE_WAITFOR(wg->wait)) {
    PLOG_WARN("waitgroup(%p) is waiting, so can't add new item");
    return -1;
  }

  proton_notify_item_t *nitem = find_group_item(wg, task);
  if (nitem != NULL) {
    PLOG_WARN("task(%ld) had append to waitgroup(%p)", ct->cid, wg);
    return 0;
  }

  PLOG_INFO("waitgroup(%p) append task(%ld)", wg, ct->cid);

  nitem = (proton_notify_item_t *)qmalloc(sizeof(proton_notify_item_t));
  nitem->group = wg;
  nitem->task = task;
  LL_insert(&nitem->wglink, wg->items.prev);
  LL_insert(&nitem->colink, ct->notify.prev);

  nitem->is_finished = ct->status == QC_STATUS_STOPED ? 1 : 0;

  // add ref-count of task
  Z_TRY_ADDREF(ct->value.myself);
  Z_TRY_ADDREF(wg->value.myself);

  wg->nitem_total++;
  wg->nitem_finished += nitem->is_finished;

  return 0;
}

int proton_wait_group_waitfor(proton_private_value_t *value) {
  proton_wait_group_t *wg = (proton_wait_group_t *)value;
  MAKESURE_PTR_NOT_NULL(wg);

  MAKESURE_ON_COROTINUE(wg->runtime);

  return proton_coroutine_waitfor(wg->runtime, &wg->wait, NULL);
}

int proton_wait_group_notify(proton_private_value_t *task, list_link_t *item) {
  proton_wait_group_t *wg =
      container_of(item, proton_notify_item_t, colink)->group;
  proton_coroutine_task *ct = (proton_coroutine_task *)task;

  MAKESURE_PTR_NOT_NULL(wg);
  MAKESURE_PTR_NOT_NULL(ct);

  if (ct->status != QC_STATUS_STOPED) {
    PLOG_WARN("wait-group input task is not stoped");
    return RC_ERROR_INVALIDATE_INPUT;
  }

  proton_notify_item_t *nitem = find_group_item(wg, task);
  if (nitem == NULL) {
    PLOG_WARN("current group(%p) is not waiting for task %ld", wg, ct->cid);
    return 0;
  }

  if (nitem->is_finished != 0) {
    PLOG_WARN("input task had stoped, skip the notify");
    return 0;
  }

  PLOG_INFO("waitgroup(%p) task(%ld) finished", wg, ct->cid);

  nitem->is_finished = 1;
  wg->nitem_finished++;

  wg->last_notify_task = nitem;

  if (IS_COROUTINE_WAITFOR(wg->wait)) {
    // some one is wait a coroutinue stoped, notify it
    if (wg->wait_mode == WG_WAIT_ONEOF) {
      proton_coroutine_wakeup(wg->runtime, &wg->wait, NULL);
    } else if (wg->wait_mode == WG_WAIT_ALL &&
               wg->nitem_finished == wg->nitem_total) {
      // all tasks had finished
      proton_coroutine_wakeup(wg->runtime, &wg->wait, NULL);
    }
  }

  return 0;
}

static proton_notify_item_t *find_group_item(proton_wait_group_t *wg,
                                             proton_private_value_t *task) {
  list_link_t *p = wg->items.next;
  while (p != &wg->items) {
    proton_notify_item_t *item = container_of(p, proton_notify_item_t, wglink);
    if (item->task == task) {

      return item;
    }

    p = p->next;
  }

  return NULL;
}

int proton_wait_group_close(proton_private_value_t *value) {
  proton_wait_group_t *wg = (proton_wait_group_t *)value;

  MAKESURE_PTR_NOT_NULL(wg);

  if (IS_COROUTINE_WAITFOR(wg->wait)) {
    PLOG_INFO("waitgroup(%p) is waiting, cancel it", wg);
    proton_coroutine_cancel(wg->runtime, &wg->wait, NULL);
  }

  list_link_t *p = wg->items.next;
  while (p != &wg->items) {
    proton_notify_item_t *item = container_of(p, proton_notify_item_t, wglink);

    // remove from task.notify queue
    LL_remove(&item->colink);

    p = LL_remove(p);

    RELEASE_VALUE_MYSELF(item->group->value);
    RELEASE_VALUE_MYSELF(*item->task);

    // release queue item
    qfree(item);
  }

  wg->nitem_finished = 0;
  wg->nitem_total = 0;
  wg->last_notify_task = NULL;

  // PLOG_INFO("waitgroup(%p) refcount=%d", wg, Z_REFCOUNT(wg->value.myself));

  return 0;
}