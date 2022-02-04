/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     tcp_server.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 02:30:36
 *
 */
#include "uvobject.h"

PROTON_TYPE_WHOAMI_DEFINE(_tcp_server_get_type, "tcpserver")

static proton_value_type_t __proton_tcpserver_type = {
    .construct = NULL,
    .destruct = proton_tcpserver_uninit,
    .whoami = _tcp_server_get_type};

proton_private_value_t *
proton_tcpserver_create(proton_coroutine_runtime *runtime) {
  proton_tcpserver_t *server =
      (proton_tcpserver_t *)qmalloc(sizeof(proton_tcpserver_t));

  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &server->tcp);
  server->value.type = &__proton_tcpserver_type;
  server->new_connection_count = 0;
  server->tcp.data = server;
  server->runtime = runtime;

  PROTON_WAIT_OBJECT_INIT(server->wq_accept);
  PROTON_WAIT_OBJECT_INIT(server->wq_close);

  return &server->value;
}

void tcpserver_on_new_connection(uv_stream_t *s, int status) {
  PLOG_DEBUG("status=%d", status);
  proton_tcpserver_t *server = (proton_tcpserver_t *)s->data;
  ++server->new_connection_count;

  if (!LL_isspin(&server->wq_accept.head)) { // some coroutine is waiting
    proton_coroutine_wakeup(server->runtime, &server->wq_accept, NULL);
  }
}

int proton_tcpserver_listen(proton_private_value_t *value, const char *host,
                            int port) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  MAKESURE_ON_COROTINUE(server->runtime);

  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  uv_tcp_bind(&server->tcp, (struct sockaddr *)&addr, 0);

  return uv_listen((uv_stream_t *)&server->tcp, 128,
                   tcpserver_on_new_connection);
}

int proton_tcpserver_accept(proton_private_value_t *value,
                            proton_private_value_t **client) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(server->runtime);

  MAKESURE_ON_COROTINUE(server->runtime);

  if (server->new_connection_count == 0) { // insert to wait queue
    proton_coroutine_waitfor(server->runtime, &server->wq_accept, NULL);
  }

  --server->new_connection_count;

  proton_tcpclient_t *c =
      (proton_tcpclient_t *)proton_tcpclient_create(server->runtime);
  int rc = uv_accept((uv_stream_t *)&server->tcp, (uv_stream_t *)&c->tcp);
  if (rc != 0) {
    PLOG_WARN("accept failed with %d", rc);
    qfree(c);
  } else {
    *client = &c->value;
  }

  return rc;
}

void tcpserver_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("closed");
  proton_tcpserver_t *server = (proton_tcpserver_t *)handle->data;

  proton_coroutine_wakeup(server->runtime, &server->wq_close, NULL);

  RELEASE_VALUE_MYSELF(server->value);
}

int proton_tcpserver_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(server->runtime);

  MAKESURE_ON_COROTINUE(server->runtime);

  if (uv_is_closing((uv_handle_t *)&server->tcp) ||
      !LL_isspin(&server->wq_close.head)) {
    PLOG_INFO("[TCPSERVER] tcpserver is closing");
    return -1;
  }

  uv_close((uv_handle_t *)&server->tcp, tcpserver_on_closed);

  proton_coroutine_waitfor(server->runtime, &server->wq_close, NULL);

  return 0;
}

int proton_tcpserver_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  if (uv_is_closing((uv_handle_t *)&server->tcp) ||
      !LL_isspin(&server->wq_close.head)) {
    PLOG_WARN("[TCPSERVER] tcpserver is closing");
    return -1;
  }
  return 0;
}