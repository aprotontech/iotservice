/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_connect.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-06 11:45:54
 *
 */

#include "http.h"
#include "php_request.h"

#define PRINT_READ_WRITE 0
#define MIN_READ_BUF_SIZE 256

PROTON_TYPE_WHOAMI_DEFINE(_http_connect_get_type, "httpconnect")

static proton_value_type_t __proton_httpconnect_type = {
    .construct = NULL,
    .destruct = proton_httpconnect_uninit,
    .whoami = _http_connect_get_type};

extern php_http_request_t *php_request_create(proton_http_connect_t *connect);
extern int http_message_parse_content(proton_http_message_t *message,
                                      char *buff, int len);

static int _httpconnect_init_read_buffer(proton_http_connect_t *connect,
                                         proton_buffer_t *buffer) {

  php_http_request_t *request = php_request_create(connect);

  connect->current = &request->message;
  connect->current->keepalive = 1; // default is keepalive

  connect->read_buffer = buffer;
  if (connect->read_buffer == NULL) {
    // alloc first buffer to recv data
    connect->read_buffer = proton_link_buffer_new_slice(
        &connect->current->buffers, HTTPCLIENT_DEFAULT_ALLOC_SIZE);
  }
  return 0;
}

static void _httpconnect_clear_ref(proton_http_connect_t *connect, int closed) {
  if (connect->current != NULL && connect->callbacks != NULL) {
    php_http_request_t *request =
        container_of(connect->current, php_http_request_t, message);

    if (Z_TYPE_P(&request->value.myself) == IS_OBJECT) {
      PLOG_DEBUG("message's refcount=%d", Z_REFCOUNT(request->value.myself));
      RELEASE_VALUE_MYSELF(request->value);
    } else {
      php_request_uninit(&request->value);
      qfree(request);
    }

    connect->current = NULL;
  }

  if (closed && connect->callbacks != NULL &&
      connect->callbacks->self != NULL) {
    // remove callbacks's self ref-count, don't use it
    RELEASE_VALUE_MYSELF(*connect->callbacks->self);
    connect->callbacks = NULL;
  }

  connect->read_buffer = NULL;
}

void httpconnect_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("onclosed");
  proton_http_connect_t *connect = (proton_http_connect_t *)handle->data;

  _httpconnect_clear_ref(connect, 1);

  if (!LL_isspin(&connect->wq_close.head)) {
    proton_coroutine_wakeup(connect->runtime, &connect->wq_close, NULL);
  }

  LL_remove(&connect->link);

  PLOG_DEBUG("httconnect's refcount=%d", Z_REFCOUNT(connect->value.myself));

  RELEASE_VALUE_MYSELF(connect->value);
}

void httpconnect_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                              uv_buf_t *buf) {
  proton_http_connect_t *connect = (proton_http_connect_t *)handle->data;
  proton_buffer_t *read_buffer = connect->read_buffer;

  if (read_buffer == NULL) {
    *buf = uv_buf_init(NULL, 0);
    PLOG_WARN("[HTTP] read buffer is null");
    return;
  }

  // if left size is too small, alloc a new buffer
  if (read_buffer->buff.len - read_buffer->used < MIN_READ_BUF_SIZE) {
    connect->read_buffer = proton_link_buffer_new_slice(
        &connect->current->buffers, HTTPCLIENT_DEFAULT_ALLOC_SIZE);
  }

  *buf = uv_buf_init(read_buffer->buff.base + read_buffer->used,
                     read_buffer->buff.len - read_buffer->used);
}

void httpconnect_on_read(uv_stream_t *handle, ssize_t nread,
                         const uv_buf_t *buf) {
  proton_http_connect_t *connect = (proton_http_connect_t *)handle->data;
#if PRINT_READ_WRITE
  PLOG_DEBUG("httpconnect(%p) read(%d), content(%s)", handle, (int)nread,
             buf->base);
#else
  PLOG_DEBUG("httpconnect(%p) read(%d)", handle, (int)nread);
#endif
  if (nread < 0) { // error
    PLOG_WARN("[HTTPCONNECT] read failed with %d", (int)nread);
    uv_close((uv_handle_t *)handle, httpconnect_on_closed);
    return;
  }

  connect->read_buffer->used += nread;

  int r = http_message_parse_content(connect->current, buf->base, nread);

  if (r < 0) {
    PLOG_WARN("[HTTPCONNECT] parse content failed, so close the connection");
    uv_read_stop((uv_stream_t *)&connect->tcp);
    uv_close((uv_handle_t *)handle, httpconnect_on_closed);
  } else if (r != nread) {
    PLOG_WARN("[HTTPCONNECT] recv more message, invalidate request");
    uv_read_stop((uv_stream_t *)&connect->tcp);
    uv_close((uv_handle_t *)handle, httpconnect_on_closed);
  } else if (connect->current->parse_finished) { // finished
    PLOG_INFO("[HTTPCONNECT] parse content done");
    proton_http_message_t *message = connect->current;
    int is_keepalive = message->keepalive;
    if (connect->type == HTTP_RESPONSE) {
      _httpconnect_clear_ref(connect, 0);
      PLOG_DEBUG("finish cleanup content");
    }
    connect->callbacks->request_handler(connect->callbacks->self, connect,
                                        message);
    if (!is_keepalive) {
      PLOG_DEBUG("[HTTPCONNECT] not keep alive");
      // uv_read_stop((uv_stream_t *)&connect->tcp);
      if (connect->type == HTTP_RESPONSE) {
        // httpclient, parse done. so close the socket
        uv_close((uv_handle_t *)handle, httpconnect_on_closed);
      }
    }
  }
}

int httpconnect_start_read(proton_http_connect_t *connect) {
  return uv_read_start((uv_stream_t *)&connect->tcp, httpconnect_alloc_buffer,
                       httpconnect_on_read);
}

int proton_http_connection_init(proton_http_connect_t *connect,
                                proton_coroutine_runtime *runtime,
                                http_connect_callbacks_t *callbacks,
                                enum http_parser_type type) {
  PLOG_DEBUG("init http connect(%p), type(%d)", connect, type);
  MAKESURE_PTR_NOT_NULL(connect);
  MAKESURE_PTR_NOT_NULL(callbacks);
  connect->value.type = &__proton_httpconnect_type;
  ZVAL_UNDEF(&connect->value.myself);
  connect->tcp.data = connect;
  connect->callbacks = callbacks;
  connect->runtime = runtime;

  LL_init(&connect->link);

  PROTON_WAIT_OBJECT_INIT(connect->wq_close);

  uv_tcp_init(RUNTIME_UV_LOOP(runtime), &connect->tcp);

  if (callbacks->self != NULL) {
    // because ref connect->callbacks.self, so add ref-count
    Z_TRY_ADDREF(callbacks->self->myself);
  }

  connect->type = type;
  connect->current = NULL;
  connect->read_buffer = NULL;
  connect->tcp_is_connected = 0;
  connect->auto_save_post_file = 0;

  return 0;
}

int proton_httpconnect_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *connect = (proton_http_connect_t *)value;
  if (!UV_HANDEL_IS_CLOSED(connect->tcp)) {
    PLOG_WARN("[HTTPCONNECT] connect is not stoped, stop it first");
    return -1;
  }

  _httpconnect_clear_ref(connect, 1);

  return 0;
}

void httpconnect_on_write(uv_write_t *req, int status) {
  proton_http_connect_t *connect = (proton_http_connect_t *)req->handle->data;
  proton_write_t *pw = (proton_write_t *)req->data;
  proton_coroutine_wakeup(connect->runtime, &pw->wq_write, NULL);
}

int httpconnect_write(proton_http_connect_t *connect, uv_buf_t rbufs[],
                      int nbufs) {
  proton_write_t pw;
  pw.writer.data = &pw;
  PROTON_WAIT_OBJECT_INIT(pw.wq_write);

#if PRINT_READ_WRITE
  for (int i = 0; i < nbufs; ++i) {
    PLOG_DEBUG("httpconnect(%p) write(%d), content(%s)", connect,
               (int)rbufs[i].len, rbufs[i].base);
  }
#endif

  int rc = uv_write(&pw.writer, (uv_stream_t *)&connect->tcp, rbufs, nbufs,
                    httpconnect_on_write);

  connect->current->write_status = PROTON_HTTP_STATUS_WRITE_DONE;

  if (rc == 0) {
    proton_coroutine_waitfor(connect->runtime, &pw.wq_write, NULL);

    if (pw.writer.error != 0) {
      rc = pw.writer.error;
    }
  }

  return rc;
}

int httpconnect_write_raw_message(proton_http_connect_t *connect,
                                  proton_link_buffer_t *lbf, const char *body,
                                  int body_len) {
  if (LL_isspin(&lbf->link)) {
    if (body != NULL) {
      uv_buf_t rbuf = {.base = (char *)body, .len = body_len};
      return httpconnect_write(connect, &rbuf, 1);
    }
  } else if (lbf->link.next == lbf->link.prev) { // only one slice
    uv_buf_t rbufs[2] = {{.base = proton_link_buffer_get_ptr(lbf, 0),
                          .len = lbf->total_used_size},
                         {.base = (char *)body, .len = body_len}};

    return httpconnect_write(connect, rbufs, body == NULL ? 1 : 2);
  } else { // a lot of slice to write
    int i = 0;
    int n = LL_size(&lbf->link) - 1;
    if (body != NULL) {
      ++n;
    }
    uv_buf_t *rbufs = (uv_buf_t *)qmalloc(n * sizeof(uv_buf_t));
    list_link_t *p = lbf->link.next;
    while (p != &lbf->link) {
      proton_buffer_t *pbt = container_of(p, proton_buffer_t, link);
      p = p->next;
      rbufs[i].base = pbt->buff.base;
      rbufs[i].len = pbt->used;
      ++i;
    }

    int rc = httpconnect_write(connect, rbufs, n);
    qfree(rbufs);
    return rc;
  }

  return 0;
}

int proton_httpconnect_close(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *connect = (proton_http_connect_t *)value;
  MAKESURE_ON_COROTINUE(connect->runtime);

  if (uv_is_closing((uv_handle_t *)&connect->tcp)) {
    PLOG_WARN("[HTTPCONNECT] connect is stoping or stoped.");
    return -1;
  }

  uv_close((uv_handle_t *)&connect->tcp, httpconnect_on_closed);

  return proton_coroutine_waitfor(connect->runtime, &connect->wq_close, NULL);
}

int httpconnect_start_message(proton_http_connect_t *connect) {
  MAKESURE_PTR_NOT_NULL(connect);
  if (connect->current != NULL) {
    if (connect->current->read_status == PROTON_HTTP_STATUS_READ_NONE &&
        connect->current->write_status == PROTON_HTTP_STATUS_WRITE_NONE) {
      // newly message
      return 0;
    }
    PLOG_WARN("connect current is process message");
    return -1;
  }

  _httpconnect_init_read_buffer(connect, NULL);
  return 0;
}

int httpconnect_finish_message(proton_http_connect_t *connect) {
  MAKESURE_PTR_NOT_NULL(connect);
  if (connect->current == NULL) {
    PLOG_WARN("connect current is not processing message");
    return -1;
  }

  int is_keepalive = connect->current->keepalive;
  _httpconnect_clear_ref(connect, 0);
  if (is_keepalive) { // renew message
    _httpconnect_init_read_buffer(connect, NULL);
  } else if (connect->type == HTTP_REQUEST) {
    // fix ab-test debug.
    PLOG_DEBUG("http request is not keepalive, so close the connection");
    uv_close((uv_handle_t *)&connect->tcp, httpconnect_on_closed);
  }
  return 0;
}

void httpconnect_on_connected(uv_connect_t *req, int status) {
  PLOG_DEBUG("connect status=%d", status);
  proton_connect_t *pc = (proton_connect_t *)req->data;
  pc->status = status;

  proton_http_connect_t *connect = (proton_http_connect_t *)req->handle->data;

  if (status == 0) {
    connect->tcp_is_connected = 1;

    httpconnect_start_read(connect);
  }

  proton_coroutine_wakeup(UV_HANDLE_RUNTIME(req->handle), &pc->wq_connect,
                          NULL);
}

int proton_httpconnect_connect(proton_http_connect_t *connect, const char *host,
                               int port) {
  if (connect->tcp_is_connected) {
    return 0;
  }

  MAKESURE_ON_COROTINUE(connect->runtime);

  PLOG_DEBUG("client connect to %s:%d", host, port);

  proton_connect_t pc = {
      .status = 0,
  };
  PROTON_WAIT_OBJECT_INIT(pc.wq_connect);
  pc.connect.data = &pc;

  struct sockaddr_in addr;
  int rc = uv_ip4_addr(host, port, &addr);
  if (rc != 0) {
    PLOG_WARN("parse ip(%s:%d) failed.", host, port);
    return -1;
  }

  rc = uv_tcp_connect(&pc.connect, &connect->tcp, (struct sockaddr *)&addr,
                      httpconnect_on_connected);
  if (rc == 0) {
    proton_coroutine_waitfor(connect->runtime, &pc.wq_connect, NULL);
    if (pc.status != 0) {
      return pc.status;
    }
  }

  return rc;
}