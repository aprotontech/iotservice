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

PROTON_TYPE_WHOAMI_DEFINE(_http_connect_get_type, "httpconnect")

static proton_value_type_t __proton_httpconnect_type = {
    .construct = NULL,
    .destruct = proton_httpconnect_uninit,
    .whoami = _http_connect_get_type};

int http_message_parse_content(proton_http_message_t *message, char *buff,
                               int len);

int _httpconnect_init_read_buffer(proton_http_connect_t *client,
                                  proton_buffer_t *buffer) {
  client->read_buffer = buffer;
  if (client->read_buffer == NULL) {
    client->read_buffer = proton_link_buffer_new_slice(
        &client->current->buffers,
        HTTPCLIENT_DEFAULT_ALLOC_SIZE); // alloc first buffer to recv data

    // mark all buffer had used
    client->read_buffer->used = client->read_buffer->buff.len;
  }
  return 0;
}

void _httpconnect_restart_parse(proton_http_connect_t *client) {
  enum http_parser_type type = client->current->parser.type;
  proton_buffer_t *buffer = client->read_buffer;
  if (!LL_isspin(&client->current->buffers.link) &&
      container_of(client->current->buffers.link.next, proton_buffer_t, link) ==
          buffer) {
    // buffer is using self-alloced content, so release it
    buffer = NULL;
  }

  http_message_uninit(client->current);
  http_message_init(client, client->current, type);
  _httpconnect_init_read_buffer(client, buffer);
}

void httpconnect_on_closed(uv_handle_t *handle) {
  PLOG_DEBUG("onclosed");
  proton_http_connect_t *client = (proton_http_connect_t *)handle->data;
  RELEASE_VALUE_MYSELF(client->value);
}

void httpconnect_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                              uv_buf_t *buf) {
  proton_http_connect_t *client = (proton_http_connect_t *)handle->data;

  *buf = client->read_buffer->buff;
}

void httpconnect_on_read(uv_stream_t *handle, ssize_t nread,
                         const uv_buf_t *buf) {
  proton_http_connect_t *client = (proton_http_connect_t *)handle->data;
  // PLOG_DEBUG("httpconnect(%p) read(%d), content(%s)", handle, (int)nread,
  //         buf->base);
  if (nread < 0) { // error
    PLOG_WARN("[HTTPCONNECT] read failed with %d", (int)nread);
    uv_close((uv_handle_t *)handle, httpconnect_on_closed);
    return;
  }

  ssize_t offset = 0;
  while (offset < nread) {
    int r = http_message_parse_content(client->current, buf->base + offset,
                                       nread - offset);
    offset += r;

    if (r < 0) {
      PLOG_WARN("[HTTPCONNECT] parse content failed, so close the connection");
      uv_read_stop((uv_stream_t *)&client->tcp);
      uv_close((uv_handle_t *)handle, httpconnect_on_closed);
      break;
    } else if (client->current->parse_finished) {         // finished
      if (client->current->parser.type == HTTP_REQUEST) { // server request
        client->callbacks->request_handler(client->callbacks->self, client,
                                           client->current);
      }
      if (client->current->keepalive) {
        //_httpconnect_restart_parse(client);
      } else { // stop read more data
        uv_read_stop((uv_stream_t *)&client->tcp);
        break;
      }
    }
  }
}

int proton_http_connection_init(proton_http_connect_t *client,
                                http_connect_callbacks_t *callbacks,
                                enum http_parser_type type) {
  PLOG_DEBUG("init http client(%p), type(%d)", client, type);
  MAKESURE_PTR_NOT_NULL(client);
  MAKESURE_PTR_NOT_NULL(callbacks);
  client->value.type = &__proton_httpconnect_type;
  client->tcp.data = client;
  client->callbacks = callbacks;

  client->current = callbacks->new_message(callbacks->self, client);

  http_message_init(client, client->current, type);

  _httpconnect_init_read_buffer(client, NULL);

  return uv_read_start((uv_stream_t *)&client->tcp, httpconnect_alloc_buffer,
                       httpconnect_on_read);
}

int proton_http_connect_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *client = (proton_http_connect_t *)value;
  if (uv_is_closing((uv_handle_t *)&client->tcp)) {
    PLOG_WARN("[HTTPCONNECT] client is closing");
    return -1;
  }

  http_message_uninit(client->current);
  return 0;
}

void httpconnect_on_write(uv_write_t *req, int status) {
  proton_http_connect_t *client = (proton_http_connect_t *)req->handle->data;
  proton_write_t *pw = (proton_write_t *)req->data;
  proton_coroutine_wakeup(client->runtime, &pw->wq_write, NULL);
}
