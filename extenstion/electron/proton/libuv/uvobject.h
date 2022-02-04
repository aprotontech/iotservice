/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     uvobject.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-16 10:14:27
 *
 */

#ifndef _PROTON_UVOBJECT_H_
#define _PROTON_UVOBJECT_H_

#include "proton/common/electron.h"
#include "proton/coroutine/runtime.h"

#include <uv.h>

#define UV_HANDLE_RUNTIME(uvhandle)                                            \
  ((proton_coroutine_runtime *)(uvhandle->loop->data))

#define UV_CURRENT_COROUTINUE(uvhandle) (UV_HANDLE_RUNTIME(uvhandle)->current)

#define MAKESURE_ON_COROTINUE(runtime)                                         \
  if (RUNTIME_CURRENT_COROUTINE(runtime) == RUNTIME_MAIN_COROUTINE(runtime)) { \
    PLOG_WARN("%s only can run on coroutine", __FUNCTION__);                   \
    return -1;                                                                 \
  }

#define RUNTIME_UV_LOOP(runtime) ((uv_loop_t *)runtime->data)

typedef struct _proton_uv_scheduler_t {
  proton_coroutine_runtime runtime;

  uv_loop_t *loop;

  uv_idle_t idle;

} proton_uv_scheduler;

typedef struct _proton_tcpserver_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;
  int new_connection_count;
  list_link_t waiting_coroutines;
  proton_coroutine_task *close_task;
} proton_tcpserver_t;

typedef struct _proton_read_context_t {
  uv_buf_t buffer;
  ssize_t filled;

  proton_coroutine_task *coroutine;
} proton_read_context_t;

typedef struct _proton_tcpclient_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;
  proton_read_context_t *reading;
  proton_coroutine_task *close_task;
} proton_tcpclient_t;

//////////// SCHEDULE
proton_uv_scheduler *proton_scheduler_create();
int proton_scheduler_free(proton_uv_scheduler *scheduler);

//////////// TCP-SERVER
proton_private_value_t *
proton_tcpserver_create(proton_coroutine_runtime *runtime);

int proton_tcpserver_listen(proton_private_value_t *value, const char *host,
                            int port);
int proton_tcpserver_accept(proton_private_value_t *value,
                            proton_private_value_t **client);

int proton_tcpserver_close(proton_private_value_t *value);

int proton_tcpserver_uninit(proton_private_value_t *value);

//////////// TCP-CLIENT
proton_private_value_t *
proton_tcpclient_create(proton_coroutine_runtime *runtime);

int proton_tcpclient_connect(proton_private_value_t *value, const char *host,
                             int port);

int proton_tcpclient_write(proton_private_value_t *value, const char *data,
                           int len);

int proton_tcpclient_read(proton_private_value_t *value, char *data, int len);

int proton_tcpclient_close(proton_private_value_t *value);

int proton_tcpclient_uninit(proton_private_value_t *value);

#endif