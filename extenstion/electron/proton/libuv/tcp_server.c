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

proton_private_value_t *
proton_tcpserver_create(quark_coroutine_runtime *runtime) {
  proton_tcpserver_t *server =
      (proton_tcpserver_t *)qmalloc(sizeof(proton_tcpserver_t));

  uv_tcp_init(runtime->loop, &server->tcp);
  server->new_connection_count = 0;
  server->tcp.data = server;

  LL_init(&server->waiting_coroutines);

  return &server->value;
}

void tcpserver_on_new_connection(uv_stream_t *s, int status) {
  QUARK_DEBUG_PRINT("");
  proton_tcpserver_t *server = (proton_tcpserver_t *)s->data;
  ++server->new_connection_count;

  if (!LL_isspin(&server->waiting_coroutines)) {
    quark_coroutine_task *task = container_of(server->waiting_coroutines.next,
                                              quark_coroutine_task, waiting);

    LL_remove(&task->waiting);     // remove from queue
    quark_coroutine_swap_in(task); // swap to it
  }
}

int proton_tcpserver_listen(proton_private_value_t *value, const char *host,
                            int port) {
  MAKE_SURE_ON_COROTINUE("listen");
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  uv_tcp_bind(&server->tcp, (struct sockaddr *)&addr, 0);

  return uv_listen((uv_stream_t *)&server->tcp, 128,
                   tcpserver_on_new_connection);
}

int proton_tcpserver_accept(proton_private_value_t *value,
                            proton_private_value_t **client) {
  MAKE_SURE_ON_COROTINUE("accept");
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();

  if (server->new_connection_count == 0) {
    // insert to wait queue
    LL_insert(&current->waiting, server->waiting_coroutines.prev);
    quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);
  }

  --server->new_connection_count;

  proton_tcpclient_t *c = (proton_tcpclient_t *)proton_tcpclient_create(
      (quark_coroutine_runtime *)server->tcp.loop->data);
  int rc = uv_accept((uv_stream_t *)&server->tcp, (uv_stream_t *)&c->tcp);
  if (rc != 0) {
    QUARK_LOGGER("accept failed with %d", rc);
    qfree(c);
  } else {
    *client = &c->value;
  }

  return rc;
}

void tcpserver_on_closed(uv_handle_t *handle) {
  QUARK_DEBUG_PRINT("closed");
  proton_tcpserver_t *server = (proton_tcpserver_t *)handle->data;
  quark_coroutine_task *task = server->close_task;

  quark_coroutine_swap_in(task);
}

int proton_tcpserver_close(proton_private_value_t *value) {
  MAKE_SURE_ON_COROTINUE("close");
  proton_tcpserver_t *server = (proton_tcpserver_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();

  if (uv_is_closing((uv_handle_t *)&server->tcp) ||
      server->close_task != NULL) {
    QUARK_LOGGER("[TCPSERVER] tcpserver is closing");
    return -1;
  }

  uv_close((uv_handle_t *)&server->tcp, tcpserver_on_closed);

  server->close_task = current;
  quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);
  server->close_task = NULL;

  return 0;
}

int proton_tcpserver_free(proton_private_value_t *value) {
  if (value != NULL) {
    proton_tcpserver_t *server = (proton_tcpserver_t *)value;
    qfree(value);
  }
  return 0;
}