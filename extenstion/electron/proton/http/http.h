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

typedef void (*on_new_http_request)(proton_private_value_t *server,
                                    proton_private_value_t *request);

typedef struct _proton_http_connect_t proton_http_connect_t;
typedef struct _proton_http_message_t proton_http_message_t;

typedef proton_http_message_t *(*connect_new_http_message_function)(
    proton_private_value_t *usr_data, proton_http_connect_t *connection);
typedef int (*connect_handle_request_function)(
    proton_private_value_t *self, proton_http_connect_t *connection,
    proton_http_message_t *message);

typedef struct _http_connect_callbacks_t {
  connect_new_http_message_function new_message;
  connect_handle_request_function request_handler;
  proton_private_value_t *self;
} http_connect_callbacks_t;

typedef struct _proton_http_server_config_t {
  char *host;
  int port;
  on_new_http_request handler;
} proton_http_server_config_t;

typedef struct _proton_http_server_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;
  list_link_t clients;
  proton_wait_object_t wq_close;
  proton_http_server_config_t config;
  http_connect_callbacks_t callbacks;
} proton_http_server_t;

typedef struct _proton_header_t {
  list_link_t link; // more than one header has same key
  char *key;
  char *value;
} proton_header_t;

typedef enum _proton_http_message_status {
  // write status
  PROTON_HTTP_STATUS_WRITE_HEAD = 0,
  PROTON_HTTP_STATUS_WRITE_BODY = 1,
  PROTON_HTTP_STATUS_WRITE_DONE = 2,

  // read status
  PROTON_HTTP_STATUS_READ_HEAD = 0,
  PROTON_HTTP_STATUS_READ_BODY = 1,
  PROTON_HTTP_STATUS_READ_DONE = 2,
} proton_http_message_status;

// typedef struct _proton_http_connect_t proton_http_connect_t;
typedef struct _proton_http_message_t {

  http_parser parser;
  http_parser_settings settings;
  char *last_header_key;
  char keepalive;
  char parse_finished;

  char *path;
  enum http_method method;

  proton_link_buffer_t buffers; // buffers

  proton_http_message_status read_status;
  proton_http_message_status write_status;

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

typedef struct _proton_http_connect_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;

  http_connect_callbacks_t *callbacks;

  proton_http_message_t *current;

  proton_buffer_t *read_buffer;

  proton_wait_object_t wq_read;

  list_link_t link;
} proton_http_connect_t;

typedef struct _proton_http_client_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  proton_http_connect_t *connect;

  proton_http_message_t message;

  http_connect_callbacks_t callbacks;
  char *host;
  int port;

} proton_http_client_t;

//////////////// HTTP-SERVER
proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config);

int proton_httpserver_start(proton_private_value_t *server);

int proton_httpserver_stop(proton_private_value_t *server);

int proton_httpserver_uninit(proton_private_value_t *server);

//////////////// HTTP-CONNECT

int proton_http_connection_init(proton_http_connect_t *client,
                                http_connect_callbacks_t *callbacks,
                                enum http_parser_type type);

int proton_httpconnect_uninit(proton_private_value_t *value);

//////////////// HTTP-CLIENT

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime, const char *host,
                         int port);

int proton_httpclient_request(proton_private_value_t *connect,
                              enum http_method method, const char *url,
                              const char *headers[], int header_count,
                              const char *body, int body_len);

//////////////// HTTP-CLIENT
int proton_httpconnect_write_response(proton_private_value_t *connect,
                                      int status_code, const char *headers[],
                                      int header_count, const char *body,
                                      int body_len);

////
void httpconnect_on_write(uv_write_t *req, int status);
int http_message_init(proton_http_connect_t *client,
                      proton_http_message_t *message,
                      enum http_parser_type type);
int http_message_uninit(proton_http_message_t *request);
#endif