/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_client.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-19 11:39:39
 *
 */
#include "http.h"

PROTON_TYPE_WHOAMI_DEFINE(_http_client_get_type, "httpclient")

static proton_value_type_t __proton_httpclient_type = {
    .construct = NULL,
    .destruct = proton_httpclient_uninit,
    .whoami = _http_client_get_type};

extern int http_message_init(proton_http_client_t *client,
                             proton_http_message_t *request,
                             enum http_parser_type type);

extern int http_message_uninit(proton_http_message_t *request);
int http_message_parse_content(proton_http_message_t *message, char *buff,
                               int len);
extern int http_message_build_response_headers(
    proton_link_buffer_t *plb, proton_http_message_t *message, int status_code,
    const char *headers[], int header_count, int body_length);

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime) {
  return NULL;
}

int _http_client_init_read_buffer(proton_http_client_t *client,
                                  proton_buffer_t *buffer) {
  client->read_buffer = buffer;
  if (client->read_buffer == NULL) {
    client->read_buffer = proton_link_buffer_new_slice(
        &client->current.buffers,
        HTTPCLIENT_DEFAULT_ALLOC_SIZE); // alloc first buffer to recv data

    // mark all buffer had used
    client->read_buffer->used = client->read_buffer->buff.len;
  }
  return 0;
}

void _httpclient_restart_parse(proton_http_client_t *client) {
  enum http_parser_type type = client->current.parser.type;
  proton_buffer_t *buffer = client->read_buffer;
  if (!LL_isspin(&client->current.buffers.link) &&
      container_of(client->current.buffers.link.next, proton_buffer_t, link) ==
          buffer) {
    // buffer is using self-alloced content, so release it
    buffer = NULL;
  }

  http_message_uninit(&client->current);
  http_message_init(client, &client->current, type);
  _http_client_init_read_buffer(client, buffer);
  client->write_status = PROTON_HTTP_STATUS_WRITE_HEAD;
  ++client->message_version;
}

void httpclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf) {
  proton_http_client_t *client = (proton_http_client_t *)handle->data;

  *buf = client->read_buffer->buff;
}

void httpclient_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("onclosed");
  proton_http_client_t *client = (proton_http_client_t *)handle->data;
  LL_remove(&((proton_sc_context_t *)client->context)->link);
  RELEASE_VALUE_MYSELF(client->value);
}

void httpclient_on_read(uv_stream_t *handle, ssize_t nread,
                        const uv_buf_t *buf) {
  proton_http_client_t *client = (proton_http_client_t *)handle->data;
  // PLOG_DEBUG("httpclient(%p) read(%d), content(%s)", handle, (int)nread,
  //         buf->base);
  if (nread < 0) { // error
    PLOG_WARN("[HTTPCLIENT] read failed with %d", (int)nread);
    uv_close((uv_handle_t *)handle, httpclient_on_closed);
    return;
  }

  ssize_t offset = 0;
  while (offset < nread) {
    int r = http_message_parse_content(&client->current, buf->base + offset,
                                       nread - offset);
    offset += r;

    if (r < 0) {
      PLOG_WARN("[HTTPCLIENT] parse content failed, so close the connection");
      uv_read_stop((uv_stream_t *)&client->tcp);
      uv_close((uv_handle_t *)handle, httpclient_on_closed);
      break;
    } else if (client->current.parse_finished) { // finished
      if (client->current.keepalive) {
        //_httpclient_restart_parse(client);
      } else { // stop read more data
        uv_read_stop((uv_stream_t *)&client->tcp);
        break;
      }
    }
  }
}

int proton_http_client_init(proton_http_client_t *client,
                            proton_buffer_t *read_buffer,
                            enum http_parser_type type) {
  PLOG_DEBUG("init http client(%p), buff(%p), type(%d)", client, read_buffer,
             type);
  client->value.type = &__proton_httpclient_type;
  client->tcp.data = client;
  client->write_status = PROTON_HTTP_STATUS_WRITE_HEAD;
  client->message_version = 0;

  http_message_init(client, &client->current, type);

  _http_client_init_read_buffer(client, read_buffer);

  return uv_read_start((uv_stream_t *)&client->tcp, httpclient_alloc_buffer,
                       httpclient_on_read);
}

int proton_httpclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_client_t *client = (proton_http_client_t *)value;
  if (uv_is_closing((uv_handle_t *)&client->tcp)) {
    PLOG_WARN("[HTTPCLIENT] client is closing");
    return -1;
  }

  http_message_uninit(&client->current);
  return 0;
}

int proton_httpclient_body_write(proton_private_value_t *value,
                                 const char *body, int len) {

  MAKESURE_PTR_NOT_NULL(value);
  proton_http_client_t *client = (proton_http_client_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  return 0;
}

void httpclient_on_write(uv_write_t *req, int status) {
  proton_http_client_t *client = (proton_http_client_t *)req->handle->data;
  proton_write_t *pw = (proton_write_t *)req->data;
  proton_coroutine_wakeup(client->runtime, &pw->wq_write, NULL);
}

int proton_httpclient_write_response(proton_private_value_t *value,
                                     int status_code, const char *headers[],
                                     int header_count, const char *body,
                                     int body_len) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_client_t *client = (proton_http_client_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  proton_link_buffer_t lbf;
  proton_link_buffer_init(&lbf, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  http_message_build_response_headers(&lbf, &client->current, status_code,
                                      headers, header_count, body_len);

  uv_buf_t rbufs[2] = {
      {.base = proton_link_buffer_get_ptr(&lbf, 0), .len = lbf.total_used_size},
      {.base = (char *)body, .len = body_len}};

  proton_write_t pw;
  pw.writer.data = &pw;
  PROTON_WAIT_OBJECT_INIT(pw.wq_write);

  int rc = uv_write(&pw.writer, (uv_stream_t *)&client->tcp, rbufs, 2,
                    httpclient_on_write);

  client->write_status = PROTON_HTTP_STATUS_WRITE_DONE;

  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pw.wq_write, NULL);

    if (pw.writer.error != 0) {
      rc = pw.writer.error;
    }
  }

  proton_link_buffer_uninit(&lbf);
  return rc;
}