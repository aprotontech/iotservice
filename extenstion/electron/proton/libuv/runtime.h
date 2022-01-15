/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     runtime.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 11:39:28
 *
 */

#ifndef __QUARK_LIBUV_RUNTIME_H__
#define __QUARK_LIBUV_RUNTIME_H__

#include "proton/include/hashmap.h"
#include "proton/include/clist.h"
#include "proton/include/electron.h"
#include "proton/coroutine/task.h"

#include <uv.h>

typedef struct _quark_coroutine_runtime_t {
  list_link_t clist;

  list_link_t runables;

  map_t cmap;

  uv_loop_t *loop;

  uv_idle_t idle;

} quark_coroutine_runtime;

quark_coroutine_runtime *quark_get_runtime();

int quark_runtime_loop(quark_coroutine_runtime *runtime);
int quark_runtime_stop(quark_coroutine_runtime *runtime);
quark_coroutine_task* quark_runtime_main(quark_coroutine_runtime *runtime);

int quark_coroutine_sleep(quark_coroutine_runtime *runtime, long time_ms);
int quark_coroutine_yield(quark_coroutine_runtime *runtime);

#endif