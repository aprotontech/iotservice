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

PROTON_TYPE_WHOAMI_DEFINE(_tcp_client_get_type, "tcpclient")

static proton_value_type_t __proton_tcpclient_type = {
    .construct = NULL,
    .destruct = proton_tcpclient_free,
    .whoami = _tcp_client_get_type};

proton_private_value_t *
proton_tcpclient_create(proton_coroutine_runtime *runtime) {
  proton_tcpclient_t *client =
      (proton_tcpclient_t *)qmalloc(sizeof(proton_tcpclient_t));

  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &client->tcp);
  client->value.type = &__proton_tcpclient_type;
  client->tcp.data = client;
  client->reading = NULL;
  client->runtime = runtime;
  client->close_task = NULL;

  return &client->value;
}

void tcpclient_on_write(uv_write_t *req, int status) {
  ((proton_coroutine_task *)req->data)->status = QC_STATUS_RUNABLE;
  proton_coroutine_resume(NULL, (proton_coroutine_task *)req->data);
}

int proton_tcpclient_write(proton_private_value_t *value, const char *data,
                           int len) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  MAKESURE_ON_COROTINUE(client->runtime);

  uv_write_t write;
  write.data = current;
  uv_buf_t buf = uv_buf_init((char *)data, (unsigned int)len);

  int rc = uv_write(&write, (uv_stream_t *)&client->tcp, &buf, 1,
                    tcpclient_on_write);
  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, value);

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
}

void tcpclient_on_read(uv_stream_t *handle, ssize_t nread,
                       const uv_buf_t *buf) {
  PLOG_DEBUG("read");
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  client->reading->filled = nread;
  uv_read_stop((uv_stream_t *)handle);

  client->reading->coroutine->status = QC_STATUS_RUNABLE;
  proton_coroutine_resume(client->runtime, client->reading->coroutine);
}

int proton_tcpclient_read(proton_private_value_t *value, char *data, int len) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  MAKESURE_ON_COROTINUE(client->runtime);

  if (client->reading != NULL) {
    PLOG_WARN("current stream is reading, can't read again");
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
    PLOG_WARN("uv_read_start failed with %d", rc);
    if (rc != UV_EALREADY) {
      PLOG_FAULT("can't reach here");
    }
    return rc;
  }

  PLOG_DEBUG("wait for read");
  proton_coroutine_waitfor(client->runtime, &client->value);
  client->reading = NULL;
  return (int)context.filled;
}

void tcpclient_on_connected(uv_connect_t *req, int status) {
  PLOG_DEBUG("connect status=%d", status);

  void **context = (void **)req->data;
  *(int *)context[1] = status;

  ((proton_coroutine_task *)context[0])->status = QC_STATUS_RUNABLE;
  proton_coroutine_resume(NULL, (proton_coroutine_task *)context[0]);
}

int proton_tcpclient_connect(proton_private_value_t *value, const char *host,
                             int port) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  int conn_status = 0;
  void *context[2] = {current, &conn_status};
  uv_connect_t connect;
  connect.data = context;

  MAKESURE_ON_COROTINUE(client->runtime);

  struct sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  int rc = uv_tcp_connect(&connect, &client->tcp, (struct sockaddr *)&addr,
                          tcpclient_on_connected);
  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &client->value);
    if (conn_status != 0) {
      return conn_status;
    }
  }

  return rc;
}

void tcpclient_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("closed");
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  proton_coroutine_task *task = client->close_task;

  RELEASE_VALUE_MYSELF(client->value);

  task->status = QC_STATUS_RUNABLE;
  proton_coroutine_resume(client->runtime, task);
}

int proton_tcpclient_close(proton_private_value_t *value) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  MAKESURE_ON_COROTINUE(client->runtime);

  if (uv_is_closing((uv_handle_t *)&client->tcp) ||
      client->close_task != NULL) {
    PLOG_INFO("[TCPCLIENT] tcpclient is closing");
    return -1;
  }

  uv_close((uv_handle_t *)&client->tcp, tcpclient_on_closed);

  client->close_task = current;
  proton_coroutine_waitfor(client->runtime, &client->value);
  client->close_task = NULL;

  return 0;
}

int proton_tcpclient_free(proton_private_value_t *value) {
  if (value != NULL) {
    proton_tcpclient_t *client = (proton_tcpclient_t *)value;
    if (client->reading) {
      PLOG_INFO("[TCPCLIENT] is reading");
    }
    qfree(value);
  }
  return 0;
}