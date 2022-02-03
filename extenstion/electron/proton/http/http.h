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

#include "proton/coroutine/runtime.h"
#include "proton/common/electron.h"
#include "proton/libuv/uvobject.h"
#include "proton/libuv/link_buff.h"
#include <uv.h>

typedef void (*on_new_http_client)(proton_private_value_t *server,
                                   proton_private_value_t *client);

typedef enum _proton_http_request_status {
  PROTON_HRC_NEW = 1,
  PROTON_HRC_SEND_HEADER = 2,
  PROTON_HRC_SEND_BODY = 3,
  PROTON_HRC_FINISH = 4,
} proton_http_request_status;

typedef struct _proton_http_server_config_t {
  char *host;
  int port;
  on_new_http_client handler;
} proton_http_server_config_t;

typedef struct _proton_http_server_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;
  list_link_t clients;
  proton_http_server_config_t config;
} proton_http_server_t;

typedef struct _proton_http_client_context_t {

} proton_http_client_context_t;

typedef struct _proton_http_request_t {

} proton_http_request_t;

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
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;

  http_parser parser;
  http_parser_settings settings;

  map_t rheaders;                // response headers map
  proton_link_buffer_t rbuffers; // buffers
  proton_link_buffer_t rbody;    // response body
  char *last_header_key;

  char keepalive;
  char rstatus;

  proton_buffer_t *read_buffer;

  proton_http_client_context_t *context;
} proton_http_client_t;

proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config);

int proton_httpserver_start(proton_private_value_t *server);

int proton_httpserver_stop(proton_private_value_t *server);

int proton_httpserver_free(proton_private_value_t *server);

int proton_httpclient_body_write(proton_private_value_t *client,
                                 const char *body, int len);

int proton_httpclient_write_response(proton_private_value_t *value,
                                     int status_code, const char *headers[],
                                     int header_count, const char *body,
                                     int body_len);

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime);

int proton_httpclient_free(proton_private_value_t *value);

#endif