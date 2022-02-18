/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_message.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-23 05:26:35
 *
 */

#include "http.h"

#define STRING_ARRAY_PARAM(s) s, sizeof(s) - 1

int httpclient_on_chunk_header(http_parser *p) { return 0; }

int httpclient_on_chunk_complete(http_parser *p) { return 0; }

int httpclient_on_headers_complete(http_parser *p) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  client->current->method = p->method;

  client->current->read_status = PROTON_HTTP_STATUS_READ_BODY;
  return 0;
}

int httpclient_on_message_begin(http_parser *p) { return 0; }

int httpclient_on_message_complete(http_parser *p) {
  PLOG_DEBUG("request read done");
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  client->current->keepalive = http_should_keep_alive(&client->current->parser);
  client->current->parse_finished = 1;

  client->current->read_status = PROTON_HTTP_STATUS_READ_DONE;

  return 0;
}

int httpclient_on_url_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  client->current->path =
      proton_link_buffer_copy_string(&client->current->buffers, at, len);
  return 0;
}

int httpclient_header_field_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  // PLOG_DEBUG("at(%s), len(%d)", at, (int)len);
  client->current->last_header_key =
      proton_link_buffer_copy_string(&client->current->buffers, at, len);

  return 0;
}

int httpclient_header_value_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  char *value =
      proton_link_buffer_copy_string(&client->current->buffers, at, len);
  // PLOG_DEBUG("at(%s), len(%d)", value, (int)len);
  char *key = client->current->last_header_key;
  client->current->last_header_key = NULL;
  if (key != NULL && value != NULL) {
    proton_header_t *new_header = (proton_header_t *)proton_link_buffer_alloc(
        &client->current->buffers, sizeof(proton_header_t),
        sizeof(proton_header_t));
    new_header->key = key;
    new_header->value = value;

    list_link_t *prev = client->current->parser.type == HTTP_REQUEST
                            ? client->current->request_headers.prev
                            : client->current->response_headers.prev;

    LL_insert(&new_header->link, prev);
  }

  return 0;
}

int httpclient_on_body_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  if (client->current->parser.type == HTTP_REQUEST) {
    proton_link_buffer_append_string(&client->current->request_body, at, len);
  } else {
    proton_link_buffer_append_string(&client->current->response_body, at, len);
  }

  return 0;
}

int http_message_init(proton_http_connect_t *client,
                      proton_http_message_t *message,
                      enum http_parser_type type) {
  http_parser_init(&message->parser, type);
  message->parser.data = client;
  message->parser.http_errno = 0;

  message->keepalive = 0;
  message->parse_finished = 0;
  message->last_header_key = NULL;

  message->write_status = PROTON_HTTP_STATUS_WRITE_NONE;
  message->read_status = PROTON_HTTP_STATUS_READ_NONE;

  /* setup callback */
  message->settings.on_message_begin = httpclient_on_message_begin;
  message->settings.on_header_field = httpclient_header_field_cb;
  message->settings.on_header_value = httpclient_header_value_cb;
  message->settings.on_url = httpclient_on_url_cb;
  message->settings.on_body = httpclient_on_body_cb;
  message->settings.on_status = NULL;
  message->settings.on_chunk_header = NULL;
  message->settings.on_chunk_complete = NULL;
  message->settings.on_headers_complete = httpclient_on_headers_complete;
  message->settings.on_message_complete = httpclient_on_message_complete;

  proton_link_buffer_init(&message->buffers, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  // request
  LL_init(&message->request_headers);
  message->request_is_chunk_mode = 0;
  proton_link_buffer_init(&message->request_body, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  // response
  message->response_is_chunk_mode = 0;
  LL_init(&message->response_headers);
  proton_link_buffer_init(&message->response_body,
                          HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  return 0;
}

int http_message_uninit(proton_http_message_t *message) {
  LL_init(&message->request_headers);
  LL_init(&message->response_headers);

  proton_link_buffer_uninit(&message->buffers);
  proton_link_buffer_uninit(&message->request_body);
  proton_link_buffer_uninit(&message->response_body);
  memset(message, 0, sizeof(proton_http_message_t));
  return 0;
}

int http_message_parse_content(proton_http_message_t *message, char *buff,
                               int len) {
  if (message->read_status == PROTON_HTTP_STATUS_READ_NONE) {
    message->read_status = PROTON_HTTP_STATUS_READ_HEAD;
  }
  size_t r =
      http_parser_execute(&message->parser, &message->settings, buff, len);
  if (message->parser.http_errno != 0) {
    PLOG_WARN("[HTTP] http_parser_execute failed with(%s:%s)",
              http_errno_name(message->parser.http_errno),
              http_errno_description(message->parser.http_errno));
    return -1;
  }

  return (int)r;
}

int http_message_build_response_headers(proton_link_buffer_t *plb,
                                        proton_http_message_t *message,
                                        int status_code, const char *headers[],
                                        int header_count, int body_length) {

  if (message->keepalive) { // http/1.1
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("HTTP/1.1 "));
  } else {
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("HTTP/1.0 "));
  }

  char buff[60];
  proton_link_buffer_append_string(
      plb, buff,
      snprintf(buff, sizeof(buff), "%03d %s\r\n", status_code,
               http_status_str((enum http_status)status_code)));
  proton_link_buffer_append_string(
      plb, STRING_ARRAY_PARAM("Server: proton/1.0\r\n"));

  if (body_length < 0) {
    proton_link_buffer_append_string(
        plb, STRING_ARRAY_PARAM("Transfer-Encoding: chunked"));
  } else {
    proton_link_buffer_append_string(
        plb, buff,
        snprintf(buff, sizeof(buff), "Content-Length: %d\r\n", body_length));
  }

  if (message->keepalive) {
    proton_link_buffer_append_string(
        plb, STRING_ARRAY_PARAM("Connection: keep-alive\r\n"));
  } else {
    proton_link_buffer_append_string(
        plb, STRING_ARRAY_PARAM("Connection: Close\r\n"));
  }

  for (int i = 0; i < header_count; ++i) {
    proton_link_buffer_append_string(plb, headers[i], strlen(headers[i]));
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("\r\n"));
  }

  proton_link_buffer_append_string(
      plb, STRING_ARRAY_PARAM("Content-Type: text/html\r\n"));
  proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("\r\n"));

  return 0;
}

int http_message_build_request_headers(proton_link_buffer_t *plb,
                                       proton_http_message_t *message,
                                       enum http_method method,
                                       const char *host, const char *headers[],
                                       int header_count, int body_length) {

  proton_link_buffer_append_string(plb, http_method_str(method),
                                   strlen(http_method_str(method)));
  proton_link_buffer_append_string(plb, " ", 1);
  proton_link_buffer_append_string(plb, message->path, strlen(message->path));

  if (message->keepalive) { // http/1.1
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM(" HTTP/1.1\r\n"));
  } else {
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM(" HTTP/1.0\r\n"));
  }

  proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("Host: "));
  proton_link_buffer_append_string(plb, host, strlen(host));
  proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("\r\n"));

  for (int i = 0; i < header_count; ++i) {
    proton_link_buffer_append_string(plb, headers[i], strlen(headers[i]));
    proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("\r\n"));
  }

  char buff[60];
  proton_link_buffer_append_string(
      plb, STRING_ARRAY_PARAM("User-Agent: proton/1.0\r\n"));

  if (method != HTTP_GET && method != HTTP_DELETE) {
    if (body_length < 0) {
      proton_link_buffer_append_string(
          plb, STRING_ARRAY_PARAM("Transfer-Encoding: chunked"));
    } else {
      proton_link_buffer_append_string(
          plb, buff,
          snprintf(buff, sizeof(buff), "Content-Length: %d\r\n", body_length));
    }
  }

  if (message->keepalive) {
    proton_link_buffer_append_string(
        plb, STRING_ARRAY_PARAM("Connection: keep-alive\r\n"));
  }

  proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("Accept: */*\r\n"));

  proton_link_buffer_append_string(plb, STRING_ARRAY_PARAM("\r\n"));

  return 0;
}

/*void _httpconnect_restart_parse(proton_http_connect_t *client) {
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
}*/