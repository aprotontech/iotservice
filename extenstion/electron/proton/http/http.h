/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-19 11:40:15
 *
 */

#ifndef _PROTON_HTTP_H_
#define _PROTON_HTTP_H_

#include "proton/libuv/runtime.h"
#include "proton/include/electron.h"
#include "proton/common/http_parser.h"
#include "proton/libuv/uvobject.h"
#include <uv.h>
#include "php.h"
#include "proton/include/electron.h"

typedef void (*on_new_http_client)(proton_private_value_t *server,
                                   proton_private_value_t *client);

typedef struct _proton_http_server_config_t {
  char *host;
  int port;
  on_new_http_client handler;
} proton_http_server_config_t;

typedef struct _proton_http_server_t {
  proton_private_value_t value;
  uv_tcp_t tcp;
  list_link_t clients;
  zval myself;
  proton_http_server_config_t config;
} proton_http_server_t;

typedef struct _proton_http_client_context_t {

} proton_http_client_context_t;

typedef struct _proton_sc_context_t {
  proton_http_client_context_t context;
  proton_http_server_t *server;
  list_link_t link;

  char *path;
  enum http_method method;
} proton_sc_context_t;

typedef struct _proton_cc_context_t {
  proton_http_client_context_t context;
  proton_http_server_t *server;
} proton_cc_context_t;

typedef struct _proton_header_t {
  list_link_t link; // more than one header has same key
  char *key;
  char *value;
} proton_header_t;

typedef struct _proton_http_client_t {
  proton_private_value_t value;
  uv_tcp_t tcp;
  http_parser parser;
  http_parser_settings settings;

  map_t rheaders;                // response headers map
  proton_link_buffer_t rbuffers; // buffers
  proton_link_buffer_t rbody;    // response body
  char *last_header_key;

  proton_buffer_t *read_buffer;

  proton_http_client_context_t *context;
} proton_http_client_t;

proton_private_value_t *
proton_httpserver_create(quark_coroutine_runtime *runtime,
                         proton_http_server_config_t *config);

int proton_httpserver_start(proton_private_value_t *server);

int proton_httpserver_stop(proton_private_value_t *server);

int proton_httpserver_free(proton_private_value_t *server);

int proton_httpclient_body_write(proton_private_value_t *server,
                                 const char *body, int len);

proton_private_value_t *
proton_httpclient_create(quark_coroutine_runtime *runtime);

int proton_httpclient_free(proton_private_value_t *value);

#endif