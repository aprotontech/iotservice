/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     tcp_client.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-15 02:30:33
 *
 */

#include "uvobject.h"

proton_private_value_t *
proton_tcpclient_create(quark_coroutine_runtime *runtime) {
  proton_tcpclient_t *client =
      (proton_tcpclient_t *)qmalloc(sizeof(proton_tcpclient_t));

  uv_tcp_init(runtime->loop, &client->tcp);
  client->tcp.data = client;
  client->reading = NULL;
  client->close_task = NULL;

  return &client->value;
}

void tcpclient_on_write(uv_write_t *req, int status) {
  quark_coroutine_swap_in((quark_coroutine_task *)req->data);
}

int proton_tcpclient_write(proton_private_value_t *value, const char *data,
                           int len) {
  MAKE_SURE_ON_COROTINUE("write");
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();
  uv_write_t write;
  write.data = current;
  uv_buf_t buf = uv_buf_init((char *)data, (unsigned int)len);

  int rc = uv_write(&write, (uv_stream_t *)&client->tcp, &buf, 1,
                    tcpclient_on_write);
  if (rc == 0) {
    quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);

    if (write.error != 0) {
      return write.error;
    }
  }

  return rc;
}

void tcpclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                            uv_buf_t *buf) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  *buf = client->reading->buffer;
  QUARK_DEBUG_PRINT("");
}

void tcpclient_on_read(uv_stream_t *handle, ssize_t nread,
                       const uv_buf_t *buf) {
  QUARK_DEBUG_PRINT("read");
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  client->reading->filled = nread;
  uv_read_stop((uv_stream_t *)handle);

  quark_coroutine_swap_in(client->reading->coroutine);
}

int proton_tcpclient_read(proton_private_value_t *value, char *data, int len) {
  MAKE_SURE_ON_COROTINUE("read");
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();

  if (client->reading != NULL) {
    QUARK_LOGGER("current stream is reading, can't read again");
    return -1;
  }

  proton_read_context_t context = {.coroutine = current,
                                   .filled = 0,
                                   .buffer =
                                       uv_buf_init(data, (unsigned int)len)};
  client->reading = &context;

  int rc = uv_read_start((uv_stream_t *)&client->tcp, tcpclient_alloc_buffer,
                         tcpclient_on_read);
  if (rc != 0) {
    QUARK_LOGGER("uv_read_start failed with %d", rc);
    if (rc != UV_EALREADY) {
      QUARK_LOGGER("can't reach here");
    }
    return rc;
  }

  QUARK_DEBUG_PRINT("wait for read");
  quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);
  client->reading = NULL;
  return (int)context.filled;
}

void tcpclient_on_connected(uv_connect_t *req, int status) {
  QUARK_DEBUG_PRINT("connect");

  void **context = (void **)req->data;
  *(int *)context[1] = status;

  quark_coroutine_swap_in((quark_coroutine_task *)context[0]);
}

int proton_tcpclient_connect(proton_private_value_t *value, const char *host,
                             int port) {
  MAKE_SURE_ON_COROTINUE("connect");
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();

  int conn_status = 0;
  void *context[2] = {current, &conn_status};
  uv_connect_t connect;
  connect.data = context;

  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  int rc = uv_tcp_connect(&connect, &client->tcp, (struct sockaddr *)&addr,
                          tcpclient_on_connected);
  if (rc == 0) {
    quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);
    if (conn_status != 0) {
      return conn_status;
    }
  }

  return rc;
}

void tcpclient_on_closed(uv_handle_t *handle) {
  QUARK_DEBUG_PRINT("closed");
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  quark_coroutine_task *task = client->close_task;

  quark_coroutine_swap_in(task);
}

int proton_tcpclient_close(proton_private_value_t *value) {
  MAKE_SURE_ON_COROTINUE("close");
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  quark_coroutine_task *current = quark_coroutine_current();

  if (uv_is_closing((uv_handle_t *)&client->tcp) ||
      client->close_task != NULL) {
    QUARK_LOGGER("[TCPCLIENT] tcpclient is closing");
    return -1;
  }

  uv_close((uv_handle_t *)&client->tcp, tcpclient_on_closed);

  client->close_task = current;
  quark_coroutine_swap_out(current, QC_STATUS_SUSPEND);
  client->close_task = NULL;

  return 0;
}

int proton_tcpclient_free(proton_private_value_t *value) {
  if (value != NULL) {
    proton_tcpclient_t *client = (proton_tcpclient_t *)value;
    if (client->reading) {
      QUARK_LOGGER("[TCPCLIENT] is reading");
    }
    qfree(value);
  }
  return 0;
}