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

extern int is_http_post_file(proton_http_message_t *message);

uv_buf_t realloc_content(proton_link_buffer_t *plb, const char *org_ptr,
                         int org_len, const char *new_ptr, int new_len) {
  char *new_buffer = proton_link_buffer_alloc(plb, org_len + new_len, 1);
  if (new_buffer != NULL) {

    memcpy(new_buffer, org_ptr, org_len);
    memcpy(new_buffer + org_len, new_ptr, new_len);

    return uv_buf_init(new_buffer, org_len + new_len);
  }
  return uv_buf_init(NULL, 0);
}

int httpclient_on_chunk_header(http_parser *p) { return 0; }

int httpclient_on_chunk_complete(http_parser *p) { return 0; }

int httpclient_on_headers_complete(http_parser *p) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  client->current->method = p->method;

  client->current->current_header = NULL;
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
  proton_http_message_t *message = client->current;
  if (message->path.base == NULL) {
    message->path = uv_buf_init((char *)at, len);
  } else {
    message->path = realloc_content(&message->buffers, message->path.base,
                                    message->path.len, at, len);
  }

  MAKESURE_PTR_NOT_NULL(message->path.base);
  return 0;
}

int httpclient_header_field_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  proton_http_message_t *message = client->current;
  proton_header_t *cur_header = message->current_header;
  // PLOG_DEBUG("at(%s), len(%d)", value, (int)len);

  if (at == NULL || (cur_header != NULL && cur_header->value.base != NULL)) {
    PLOG_DEBUG("new header started, ptr(%p)", at);
    cur_header = NULL;
  }

  if (cur_header == NULL) {
    cur_header = (proton_header_t *)proton_link_buffer_alloc(
        &message->buffers, sizeof(proton_header_t), sizeof(proton_header_t));
    MAKESURE_PTR_NOT_NULL(cur_header);
    cur_header->key = uv_buf_init((char *)at, len);
    cur_header->value = uv_buf_init(NULL, 0);

    list_link_t *prev = message->headers.prev;
    LL_insert(&cur_header->link, prev);
  } else {
    cur_header->key = realloc_content(&message->buffers, cur_header->key.base,
                                      cur_header->key.len, at, len);

    MAKESURE_PTR_NOT_NULL(cur_header->key.base);
  }

  message->current_header = cur_header;

  return 0;
}

int httpclient_header_value_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  proton_http_message_t *message = client->current;
  proton_header_t *cur_header = message->current_header;
  // PLOG_DEBUG("at(%s), len(%d)", value, (int)len);

  if (cur_header->value.base == NULL) {
    cur_header->value = uv_buf_init((char *)at, len);
  } else {
    cur_header->value =
        realloc_content(&message->buffers, cur_header->value.base,
                        cur_header->value.len, at, len);

    MAKESURE_PTR_NOT_NULL(cur_header->value.base);
  }

  return 0;
}

int httpclient_on_body_cb(http_parser *p, const char *at, size_t len) {
  proton_http_connect_t *client = (proton_http_connect_t *)p->data;
  proton_http_message_t *message = client->current;

  proton_buffer_t *buffer = (proton_buffer_t *)proton_link_buffer_alloc(
      &message->buffers, sizeof(proton_buffer_t), sizeof(proton_buffer_t));
  MAKESURE_PTR_NOT_NULL(buffer);
  buffer->buff = uv_buf_init((char *)at, len);
  buffer->used = len;
  buffer->need_free = 0;
  LL_init(&buffer->link);

  proton_link_buffer_append_slice(&message->body, buffer);

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
  message->current_header = NULL;
  message->path = uv_buf_init(NULL, 0);

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

  // request/response
  message->is_chunk_mode = 0;
  LL_init(&message->headers);
  proton_link_buffer_init(&message->body, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);
  message->raw_body = NULL;
  message->content_type = PROTON_HTTP_CT_NORMAL;

  LL_init(&message->multiparts);
  message->boundary = uv_buf_init(NULL, 0);
  message->is_parsed_multipart = 0;

  return 0;
}

int http_message_uninit(proton_http_message_t *message) {
  LL_init(&message->headers);

  proton_link_buffer_uninit(&message->buffers);
  proton_link_buffer_uninit(&message->body);

  if (message->raw_body != NULL) {
    zend_string_release(message->raw_body);
  }

  list_link_t *p = message->multiparts.next;
  while (p != &message->multiparts) {
    proton_multipart_t *mp = container_of(p, proton_multipart_t, link);
    p = p->next;
    if (mp->is_file && mp->tmp_name.base != NULL && mp->tmp_name.len != 0) {
      int ret = unlink(mp->tmp_name.base);
      PLOG_INFO("remove temp file(%s) ret(%d)", mp->tmp_name.base, ret);
    }
  }
  LL_init(&message->multiparts);

  memset(message, 0, sizeof(proton_http_message_t));
  return 0;
}

zend_string *http_message_get_raw_body(proton_http_message_t *message) {
  if (message != NULL) {
    if (message->raw_body == NULL) {
      message->raw_body = proton_link_to_string(&message->body);
    }

    return message->raw_body;
  }

  return NULL;
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
  proton_link_buffer_append_string(plb, message->path.base, message->path.len);

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
