/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_server.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-19 11:39:43
 *
 */
#include "http.h"
#include "php_request.h"

PROTON_TYPE_WHOAMI_DEFINE(_http_server_get_type, "httpserver")

static proton_value_type_t __proton_httpserver_type = {
    .construct = NULL,
    .destruct = proton_httpserver_uninit,
    .whoami = _http_server_get_type};

int http_server_request_handler(proton_private_value_t *self,
                                proton_http_connect_t *connection,
                                proton_http_message_t *message) {
  php_http_request_t *request =
      container_of(message, php_http_request_t, message);
  ((proton_http_server_t *)self)->config.handler(self, &request->value);
  return 0;
}

proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config) {
  if (config == NULL || config->handler == NULL || config->host == NULL) {
    PLOG_WARN("invalidate input config");
    return NULL;
  }

  int host_len = strlen(config->host);
  proton_http_server_t *server = (proton_http_server_t *)qmalloc(
      sizeof(proton_http_server_t) + host_len + 1);

  LL_init(&server->clients);

  server->config = *config;
  server->config.host = (char *)&server[1];
  strcpy(server->config.host, config->host);

  server->callbacks.request_handler = http_server_request_handler;
  server->callbacks.self = &server->value;

  uv_tcp_init((uv_loop_t *)runtime->data, &server->tcp);
  server->tcp.data = server;

  struct sockaddr_in addr;
  uv_ip4_addr(server->config.host, server->config.port, &addr);
  int rc = uv_tcp_bind(&server->tcp, (struct sockaddr *)&addr, 0);
  if (rc != 0) {
    PLOG_WARN("[TCPSERVER] server bind(%s:%d) failed with [%d](%s).",
              server->config.host, server->config.port, rc, uv_err_name(rc));
    return NULL;
  }

  server->runtime = runtime;
  server->value.type = &__proton_httpserver_type;

  PROTON_WAIT_OBJECT_INIT(server->wq_close);

  return &server->value;
}

void httpserver_on_new_connection(uv_stream_t *s, int status) {
  proton_http_server_t *server = (proton_http_server_t *)s->data;

  proton_http_connect_t *connect =
      (proton_http_connect_t *)qmalloc(sizeof(proton_http_connect_t));

  proton_http_connection_init(connect, server->runtime, &server->callbacks,
                              HTTP_REQUEST);

  int rc = uv_accept((uv_stream_t *)&server->tcp, (uv_stream_t *)&connect->tcp);
  if (rc != 0) { // acception new client failed
    PLOG_WARN("[HTTPSERVER] accept new client failed! err=%d", rc);
    qfree(connect);
    return;
  }

  // start read
  httpconnect_start_message(connect);
  httpconnect_start_read(connect);

  LL_insert(&connect->link, server->clients.prev);
}

int proton_httpserver_start(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_server_t *server = (proton_http_server_t *)value;
  MAKESURE_ON_COROTINUE(server->runtime);
  return uv_listen((uv_stream_t *)&server->tcp, 128,
                   httpserver_on_new_connection);
}

void httpserver_on_closed(uv_handle_t *handle) {
  proton_http_server_t *server = (proton_http_server_t *)handle->data;

  PLOG_DEBUG("server closed");

  proton_coroutine_wakeup(server->runtime, &server->wq_close, NULL);

  RELEASE_VALUE_MYSELF(server->value);
}

int proton_httpserver_stop(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_server_t *server = (proton_http_server_t *)value;
  MAKESURE_ON_COROTINUE(server->runtime);

  PLOG_DEBUG("[HTTPSERVER] stop httpserver");

  if (uv_is_closing((uv_handle_t *)&server->tcp) ||
      !LL_isspin(&server->wq_close.head)) {
    PLOG_INFO("[HTTPSERVER] httpserver is closing");
    return -1;
  }

  /// close all clients

  uv_close((uv_handle_t *)&server->tcp, httpserver_on_closed);

  proton_coroutine_waitfor(server->runtime, &server->wq_close, NULL);
}

int proton_httpserver_uninit(proton_private_value_t *value) {
  proton_http_server_t *server = (proton_http_server_t *)value;
  if (!UV_HANDEL_IS_CLOSED(server->tcp)) {
    PLOG_WARN("[HTTPSERVER] server is not closed, stop it first");
    return -1;
  }

  return 0;
}
