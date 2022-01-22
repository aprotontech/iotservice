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

#define HTTPCLIENT_DEFAULT_ALLOC_SIZE 4096
#define HTTPCLIENT_MAX_BUFFER_SIZE (2 * 1024 * 1024)

proton_private_value_t *
proton_httpclient_create(quark_coroutine_runtime *runtime) {
  return NULL;
}

int proton_httpclient_free(proton_private_value_t *value) { return 0; }

void httpclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf) {
  proton_http_client_t *client = (proton_http_client_t *)handle->data;

  *buf = client->read_buffer->buff;
}

void httpclient_on_closed(uv_handle_t *handle) { QUARK_DEBUG_PRINT("closed"); }

void httpclient_on_read(uv_stream_t *handle, ssize_t nread,
                        const uv_buf_t *buf) {
  proton_http_client_t *client = (proton_http_client_t *)handle->data;
  if (nread < 0) { // error
    QUARK_LOGGER("[HTTPCLIENT] read failed with %d", (int)nread);
    uv_close((uv_handle_t *)handle, httpclient_on_closed);
    return;
  }

  size_t r =
      http_parser_execute(&client->parser, &client->settings, buf->base, nread);

  if (client->parser.http_errno) {
    QUARK_LOGGER("[HTTPSERVER] http_parser_execute failed with(%s:%s)",
                 http_errno_name(client->parser.http_errno),
                 http_errno_description(client->parser.http_errno));
    uv_close((uv_handle_t *)handle, httpclient_on_closed);
    return;
  }
}

int httpclient_on_chunk_header(http_parser *p) { return 0; }

int httpclient_on_chunk_complete(http_parser *p) { return 0; }

int httpclient_on_headers_complete(http_parser *p) { return 0; }

int httpclient_on_message_begin(http_parser *p) { return 0; }

int httpclient_on_message_complete(http_parser *p) {
  proton_http_client_t *client = (proton_http_client_t *)p->data;
  if (p->type == HTTP_REQUEST) { // server request
    proton_sc_context_t *context = (proton_sc_context_t *)client->context;
    context->server->config.handler(&context->server->value, &client->value);
  }

  return 0;
}

int httpclient_on_url_cb(http_parser *p, const char *at, size_t len) {
  proton_http_client_t *client = (proton_http_client_t *)p->data;
  proton_sc_context_t *context = (proton_sc_context_t *)client->context;
  context->path = proton_link_buffer_copy_string(&client->rbuffers, at, len);
  return 0;
}

int httpclient_header_field_cb(http_parser *p, const char *at, size_t len) {
  proton_http_client_t *client = (proton_http_client_t *)p->data;
  client->last_header_key =
      proton_link_buffer_copy_string(&client->rbuffers, at, len);
  return 0;
}

int httpclient_header_value_cb(http_parser *p, const char *at, size_t len) {
  proton_http_client_t *client = (proton_http_client_t *)p->data;
  char *value = proton_link_buffer_copy_string(&client->rbuffers, at, len);

  char *key = client->last_header_key;
  client->last_header_key = NULL;
  if (key != NULL && value != NULL) {
    proton_header_t *new_header = (proton_header_t *)proton_link_buffer_alloc(
        &client->rbuffers, sizeof(proton_header_t), sizeof(proton_header_t));
    new_header->key = key;
    new_header->value = value;

    any_t header = NULL;
    if (hashmap_get(client->rheaders, key, &header) == MAP_OK) {
      LL_insert(&new_header->link, ((proton_header_t *)header)->link.prev);
    } else {
      LL_init(&new_header->link);
      hashmap_put(client->rheaders, key, new_header);
    }
  }

  return 0;
}

int httpclient_on_body_cb(http_parser *p, const char *at, size_t len) {
  proton_http_client_t *client = (proton_http_client_t *)p->data;
  proton_link_buffer_append_string(&client->rbody, at, len);
}

int proton_http_client_init(proton_http_client_t *client,
                            enum http_parser_type type) {
  client->tcp.data = client;
  proton_link_buffer_init(&client->rbody, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);
  proton_link_buffer_init(&client->rbuffers, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);
  client->read_buffer = proton_link_buffer_new_slice(
      &client->rbuffers,
      HTTPCLIENT_DEFAULT_ALLOC_SIZE); // alloc first buffer to recv data

  client->read_buffer->used =
      client->read_buffer->buff.len; // mark all buffer had used
  client->last_header_key = NULL;
  client->rheaders = hashmap_new();
  // LL_init(&client->rheaders);
  http_parser_init(&client->parser, type);
  client->parser.data = client;
  client->parser.http_errno = 0;

  /* setup callback */
  client->settings.on_message_begin = httpclient_on_message_begin;
  client->settings.on_header_field = httpclient_header_field_cb;
  client->settings.on_header_value = httpclient_header_value_cb;
  client->settings.on_url = httpclient_on_url_cb;
  client->settings.on_body = httpclient_on_body_cb;
  client->settings.on_headers_complete = httpclient_on_headers_complete;
  client->settings.on_message_complete = httpclient_on_message_complete;

  return uv_read_start((uv_stream_t *)&client->tcp, httpclient_alloc_buffer,
                       httpclient_on_read);
}

int proton_httpclient_body_write(proton_private_value_t *server,
                                 const char *body, int len) {
  MAKE_SURE_ON_COROTINUE("write");
  return 0;
}