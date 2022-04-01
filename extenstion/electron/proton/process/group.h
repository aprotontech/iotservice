/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     group.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-03-29 10:44:14
 *
 */

#ifndef _ELECTRON_PROCESS_H_
#define _ELECTRON_PROCESS_H_

#include "proton/libuv/uvobject.h"

#define MAX_WATCH_PATH_LENGTH 256

typedef int (*on_worker_new_client)(proton_private_value_t *server,
                                    uv_stream_t *q);

typedef enum _proton_process_type {
  PROCESS_MASTER,
  PROCESS_WORKER
} proton_process_type;

typedef enum _proton_load_balance_type {
  LOAD_BALANCE_RANDOM = 0,
  LOAD_BALANCE_CUSTOM = 2,
} proton_load_balnace_type;

typedef enum _proton_process_group_status {
  PG_STATUS_INITED,
  PG_STATUS_RUNNING,
  PG_STATUS_STOPED,
} proton_process_group_status;

typedef struct _proton_process_group_config_t {
  uint64_t max_requests;
  int enable_dynamic_worker;
  uint32_t static_worker_count;
  uint32_t max_idle_worker_count;

  char watch_path[MAX_WATCH_PATH_LENGTH];

  proton_load_balnace_type lb_type;
  zval custom_load_balance;

} proton_process_group_config_t;

typedef struct _proton_process_worker_t {
  list_link_t link;
  uv_process_t req;
  uv_process_options_t options;
  pid_t pid;
  uv_pipe_t pipe;
  uint64_t requests_count;
  int is_running;

  // connections
  list_link_t connections;
} proton_process_worker_t;

typedef struct _proton_process_group_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  proton_process_group_config_t config;

  proton_process_type type;
  proton_process_group_status status;

  uv_signal_t signal_child;

  proton_private_value_t *server;
  on_worker_new_client on_new_client;

  proton_process_worker_t *current;

  int worker_count;
  proton_process_worker_t *workers;
} proton_process_group_t;

int proton_process_group_init(proton_private_value_t *group,
                              proton_coroutine_runtime *runtime,
                              proton_process_group_config_t *config);

int proton_process_group_get_type(proton_private_value_t *group);

int proton_process_group_uninit(proton_private_value_t *group);

int proton_process_group_bind_server(proton_private_value_t *group,
                                     proton_private_value_t *server,
                                     on_worker_new_client callback);

int proton_process_group_start(proton_private_value_t *group);

int proton_process_group_stop(proton_private_value_t *group);

int proton_process_new_tcpclient(proton_private_value_t *group,
                                 proton_private_value_t *server,
                                 uv_stream_t *server_stream);

#endif