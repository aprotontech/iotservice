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

#include "http_message.h"

#define HTTPCLIENT_DEFAULT_ALLOC_SIZE 4096
#define HTTPCLIENT_MAX_BUFFER_SIZE (2 * 1024 * 1024)

typedef void (*on_new_http_request)(proton_private_value_t *server,
                                    proton_private_value_t *request);

typedef struct _proton_http_connect_t proton_http_connect_t;
typedef struct _proton_http_message_t proton_http_message_t;

typedef int (*connect_handle_request_function)(
    proton_private_value_t *self, proton_http_connect_t *connection,
    proton_http_message_t *message);

typedef struct _http_connect_callbacks_t {
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

typedef struct _proton_http_connect_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;
  uv_tcp_t tcp;

  enum http_parser_type type;
  int tcp_is_connected;

  http_connect_callbacks_t *callbacks;

  proton_http_message_t *current;

  proton_buffer_t *read_buffer;

  proton_wait_object_t wq_close;

  list_link_t link;
} proton_http_connect_t;

typedef struct _proton_http_client_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  proton_http_connect_t *connect;

  http_connect_callbacks_t callbacks;
  char *host;
  int port;
  int is_https;

} proton_http_client_t;

//////////////// HTTP-SERVER
proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config);

int proton_httpserver_start(proton_private_value_t *server);

int proton_httpserver_stop(proton_private_value_t *server);

int proton_httpserver_uninit(proton_private_value_t *server);

//////////////// HTTP-CONNECT

int proton_http_connection_init(proton_http_connect_t *connect,
                                proton_coroutine_runtime *runtime,
                                http_connect_callbacks_t *callbacks,
                                enum http_parser_type type);

int proton_httpconnect_uninit(proton_private_value_t *connect);

int proton_httpconnect_close(proton_private_value_t *connect);

//////////////// HTTP-CLIENT

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime, const char *host,
                         int port);

int proton_httpclient_request(proton_private_value_t *client,
                              enum http_method method, const char *url,
                              const char *headers[], int header_count,
                              const char *body, int body_len);

int proton_httpclient_uninit(proton_private_value_t *client);

//////////////// HTTP-CONNECT

int proton_httpconnect_connect(proton_http_connect_t *connect, const char *host,
                               int port);
int proton_httpconnect_write_response(proton_private_value_t *connect,
                                      int status_code, const char *headers[],
                                      int header_count, const char *body,
                                      int body_len);

////
int httpconnect_start_read(proton_http_connect_t *connect);
int httpconnect_start_message(proton_http_connect_t *connect);
int httpconnect_finish_message(proton_http_connect_t *connect);
int httpconnect_write_raw_message(proton_http_connect_t *connect,
                                  proton_link_buffer_t *lbf, const char *body,
                                  int body_len);

#endif