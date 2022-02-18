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
    .destruct = proton_tcpclient_uninit,
    .whoami = _tcp_client_get_type};

proton_private_value_t *
proton_tcpclient_create(proton_coroutine_runtime *runtime) {
  proton_tcpclient_t *client =
      (proton_tcpclient_t *)qmalloc(sizeof(proton_tcpclient_t));

  client->value.type = &__proton_tcpclient_type;
  ZVAL_UNDEF(&client->value.myself);

  client->reading = NULL;
  client->runtime = runtime;

  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &client->tcp);
  client->tcp.data = client;

  PROTON_WAIT_OBJECT_INIT(client->wq_close);

  return &client->value;
}

void tcpclient_on_write(uv_write_t *req, int status) {
  PLOG_DEBUG("write status=%d", status);
  proton_tcpclient_t *client = (proton_tcpclient_t *)req->handle->data;
  proton_write_t *pw = (proton_write_t *)req->data;
  proton_coroutine_wakeup(client->runtime, &pw->wq_write, NULL);
}

int proton_tcpclient_write(proton_private_value_t *value, const char *data,
                           int len) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  MAKESURE_ON_COROTINUE(client->runtime);

  proton_write_t pw;
  pw.writer.data = &pw;
  PROTON_WAIT_OBJECT_INIT(pw.wq_write);
  uv_buf_t buf = uv_buf_init((char *)data, (unsigned int)len);

  int rc = uv_write(&pw.writer, (uv_stream_t *)&client->tcp, &buf, 1,
                    tcpclient_on_write);
  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pw.wq_write, NULL);

    if (pw.writer.error != 0) {
      rc = pw.writer.error;
      PLOG_WARN("client(%p) write failed with error[%d](%s)", client, rc,
                uv_err_name(rc));
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
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;
  client->reading->filled = nread;
  uv_read_stop((uv_stream_t *)handle);

  proton_coroutine_wakeup(client->runtime, &client->reading->wq_read, NULL);
}

int proton_tcpclient_read(proton_private_value_t *value, char *data, int len) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  proton_coroutine_task *current = RUNTIME_CURRENT_COROUTINE(client->runtime);

  MAKESURE_ON_COROTINUE(client->runtime);

  if (client->reading != NULL) {
    PLOG_WARN("[TCPCLIENT] current stream is reading, can't read again");
    return -1;
  }

  proton_read_context_t context = {
      .filled = 0, .buffer = uv_buf_init(data, (unsigned int)len)};
  PROTON_WAIT_OBJECT_INIT(context.wq_read);
  client->reading = &context;

  int rc = uv_read_start((uv_stream_t *)&client->tcp, tcpclient_alloc_buffer,
                         tcpclient_on_read);
  if (rc != 0) {
    PLOG_WARN("[TCPCLIENT] uv_read_start failed with %d", rc);
    if (rc != UV_EALREADY) {
      PLOG_FAULT("can't reach here");
    }
    return rc;
  }

  PLOG_DEBUG("wait for read");
  proton_coroutine_waitfor(client->runtime, &context.wq_read, NULL);
  client->reading = NULL;
  return (int)context.filled;
}

void tcpclient_on_connected(uv_connect_t *req, int status) {
  PLOG_DEBUG("connect status=%d", status);
  proton_connect_t *pc = (proton_connect_t *)req->data;
  pc->status = status;

  proton_coroutine_wakeup(UV_HANDLE_RUNTIME(req->handle), &pc->wq_connect,
                          NULL);
}

int proton_tcpclient_connect(proton_private_value_t *value, const char *host,
                             int port) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(host);
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  proton_connect_t pc = {
      .status = 0,
  };
  PROTON_WAIT_OBJECT_INIT(pc.wq_connect);
  pc.connect.data = &pc;

  struct sockaddr_in addr;
  int rc = uv_ip4_addr(host, port, &addr);
  if (rc != 0) {
    PLOG_WARN("parse ip(%s) failed.", host);
    return -1;
  }

  rc = uv_tcp_connect(&pc.connect, &client->tcp, (struct sockaddr *)&addr,
                      tcpclient_on_connected);
  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pc.wq_connect, NULL);
    if (pc.status != 0) {
      return pc.status;
    }
  }

  return rc;
}

void tcpclient_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("closed");
  proton_tcpclient_t *client = (proton_tcpclient_t *)handle->data;

  proton_coroutine_wakeup(client->runtime, &client->wq_close, NULL);

  PLOG_DEBUG("tcpclient(%p) refcount=%d", client,
             Z_REFCOUNT(client->value.myself) - 1);

  RELEASE_VALUE_MYSELF(client->value);
}

int proton_tcpclient_close(proton_private_value_t *value) {
  proton_tcpclient_t *client = (proton_tcpclient_t *)value;

  MAKESURE_ON_COROTINUE(client->runtime);

  if (uv_is_closing((uv_handle_t *)&client->tcp) ||
      !LL_isspin(&client->wq_close.head)) {
    PLOG_INFO("[TCPCLIENT] tcpclient is closing");
    return -1;
  }

  uv_close((uv_handle_t *)&client->tcp, tcpclient_on_closed);

  proton_coroutine_waitfor(client->runtime, &client->wq_close, NULL);

  return 0;
}

int proton_tcpclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);

  proton_tcpclient_t *client = (proton_tcpclient_t *)value;
  if (client->reading) {
    PLOG_INFO("[TCPCLIENT] tcpclient is reading");
    return -1;
  }

  if (!LL_isspin(&client->wq_close.head)) {
    PLOG_WARN("[TCPCLIENT] tcpclient is closing");
    return -1;
  }

  return 0;
}