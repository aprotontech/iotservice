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

#define HTTPCLIENT_DEFAULT_ALLOC_SIZE 4096
#define HTTPCLIENT_MAX_BUFFER_SIZE (2 * 1024 * 1024)

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
  proton_wait_object_t wq_close;
  proton_http_server_config_t config;
} proton_http_server_t;

typedef struct _proton_http_client_context_t {

} proton_http_client_context_t;

typedef enum _proton_http_message_status {
  PROTON_HTTP_STATUS_WRITE_HEAD = 0,
  PROTON_HTTP_STATUS_WRITE_BODY = 1,
  PROTON_HTTP_STATUS_WRITE_DONE = 2,
} proton_http_message_status;

// typedef struct _proton_http_client_t proton_http_client_t;
typedef struct _proton_http_message_t {

  http_parser parser;
  http_parser_settings settings;
  char *last_header_key;
  char keepalive;
  char parse_finished;

  char *path;
  enum http_method method;

  proton_link_buffer_t buffers; // buffers

  ///////////// REQUEST
  // request headers map(item: proton_header_t)
  map_t request_headers;
  char request_is_chunk_mode;
  proton_link_buffer_t request_body; // request body

  ///////////// RESPONSE
  char response_is_chunk_mode;
  map_t response_headers;
  proton_link_buffer_t response_body; // response body

} proton_http_message_t;

typedef struct _proton_sc_context_t {
  proton_http_client_context_t context;
  proton_http_server_t *server;
  list_link_t link;

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

  proton_http_message_t current;
  uint64_t message_version;

  proton_buffer_t *read_buffer;

  proton_http_message_status write_status;

  proton_wait_object_t wq_read;

  proton_http_client_context_t *context;

} proton_http_client_t;

//////////////// HTTP-SERVER
proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config);

int proton_httpserver_start(proton_private_value_t *server);

int proton_httpserver_stop(proton_private_value_t *server);

int proton_httpserver_uninit(proton_private_value_t *server);

//////////////// HTTP-CLIENT

int proton_httpclient_body_write(proton_private_value_t *client,
                                 const char *body, int len);

int proton_httpclient_write_response(proton_private_value_t *value,
                                     int status_code, const char *headers[],
                                     int header_count, const char *body,
                                     int body_len);

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime);

int proton_httpclient_uninit(proton_private_value_t *value);

#endif