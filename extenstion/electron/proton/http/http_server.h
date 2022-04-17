/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_server.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-12 10:07:11
 *
 */

#ifndef _PROTON_HTTP_SERVER_H_
#define _PROTON_HTTP_SERVER_H_

#include "http.h"

typedef struct _proton_http_server_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;
  list_link_t clients;
  proton_wait_object_t wq_close;
  proton_http_server_config_t config;
  http_connect_callbacks_t callbacks;
  proton_private_value_t *process_group;
} proton_http_server_t;

#endif