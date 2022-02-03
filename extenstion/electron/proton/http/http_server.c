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

typedef struct _proton_http_server_client_wrap_t {
  proton_http_client_t client;
  proton_sc_context_t context;
} proton_http_server_client_wrap_t;

proton_private_value_t *
proton_httpserver_create(proton_coroutine_runtime *runtime,
                         proton_http_server_config_t *config) {
  assert(config != NULL);
  assert(config->handler != NULL);
  assert(config->host != NULL);

  int host_len = strlen(config->host);
  proton_http_server_t *server = (proton_http_server_t *)qmalloc(
      sizeof(proton_http_server_t) + host_len + 1);

  LL_init(&server->clients);

  server->config = *config;
  server->config.host = (char *)&server[1];
  strcpy(server->config.host, config->host);

  uv_tcp_init((uv_loop_t *)runtime->data, &server->tcp);
  server->tcp.data = server;

  struct sockaddr_in addr;
  uv_ip4_addr(server->config.host, server->config.port, &addr);
  uv_tcp_bind(&server->tcp, (struct sockaddr *)&addr, 0);

  server->runtime = runtime;

  return &server->value;
}

extern int proton_http_client_init(proton_http_client_t *client,
                                   enum http_parser_type type);
void httpserver_on_new_connection(uv_stream_t *s, int status) {
  proton_http_server_t *server = (proton_http_server_t *)s->data;
  proton_http_server_client_wrap_t *wrap =
      (proton_http_server_client_wrap_t *)qmalloc(
          sizeof(proton_http_server_client_wrap_t));
  proton_http_client_t *client = &wrap->client;

  uv_tcp_init(server->tcp.loop, &client->tcp);
  int rc = uv_accept((uv_stream_t *)&server->tcp, (uv_stream_t *)&client->tcp);
  if (rc != 0) { // acception new client failed
    PLOG_WARN("[HTTPSERVER] accept new client failed! err=%d", rc);
    qfree(client);
    return;
  }

  client->context = &wrap->context.context;
  client->runtime = server->runtime;
  proton_http_client_init(client, HTTP_REQUEST);

  // append to list
  wrap->context.server = server;
  LL_insert(&wrap->context.link, server->clients.prev);
}

int proton_httpserver_start(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  MAKESURE_ON_COROTINUE(server->runtime);
  return uv_listen((uv_stream_t *)&server->tcp, 128,
                   httpserver_on_new_connection);
}

int proton_httpserver_stop(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  MAKESURE_ON_COROTINUE(server->runtime);
  return 0;
}

int proton_httpserver_free(proton_private_value_t *server) { return 0; }