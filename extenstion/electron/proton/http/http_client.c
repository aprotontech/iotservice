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

extern void _httpconnect_restart_parse(proton_http_connect_t *client);

typedef struct _http_client_one_connection_t {
  proton_http_client_t client;
  proton_http_connect_t connection;
} http_client_one_connection_t;

typedef struct _http_client_url_info_t {
  int is_https;
  int port;
  char *host;
  char *path;
} http_client_url_info_t;

proton_http_message_t *
http_client_reuse_request_message(proton_private_value_t *self,
                                  proton_http_connect_t *connection) {
  proton_http_message_t *message = &((proton_http_client_t *)self)->message;
  http_message_uninit(message);

  http_message_init(connection, message, HTTP_RESPONSE);
  return message;
}

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime, const char *host,
                         int port) {
  int host_len = host != NULL ? strlen(host) : 0;
  http_client_one_connection_t *cc = (http_client_one_connection_t *)qmalloc(
      sizeof(http_client_one_connection_t) + strlen(host) + 1);
  cc->client.connect = &cc->connection;
  cc->client.runtime = runtime;
  cc->client.host = NULL;
  if (host != NULL) {
    cc->client.host = (char *)(&cc[1]);
    strcpy(cc->client.host, host);
  }
  cc->client.callbacks.self = &cc->client.value;
  cc->client.callbacks.new_message = http_client_reuse_request_message;
  cc->client.callbacks.request_handler = NULL;

  return &cc->client.value;
}

http_client_url_info_t *http_client_parse_url(proton_http_client_t *client,
                                              const char *raw_url) {
  struct http_parser_url url;
  int ret = http_parser_parse_url(raw_url, strlen(raw_url), 0, &url);
  if (ret != 0) {
    PLOG_DEBUG("http_parser_parse_url(%s) failed,ret(%d)", raw_url, ret);
    return NULL;
  }

  if (!(url.field_set & (1 << UF_HOST))) {
    PLOG_DEBUG("http_parser_parse_url(%s) failed, not found host", raw_url);
    return NULL;
  }

  // default values
  int out_is_https = 0;
  int out_port = 80;

  if (url.field_set & (1 << UF_SCHEMA)) {
    char _schema[20] = {0};
    if (url.field_data[UF_SCHEMA].len >= sizeof(_schema) - 1) {
      PLOG_DEBUG("http_parser_parse_url(%s) failed, schema is invalidate",
                 raw_url);
      return NULL;
    }
    memcpy(_schema, raw_url + url.field_data[UF_SCHEMA].off,
           url.field_data[UF_SCHEMA].len);
    for (ret = 0; _schema[ret] != '\0'; ++ret) {
      if (_schema[ret] >= 'A' && _schema[ret] <= 'Z') {
        _schema[ret] += 32; // to lower
      }
    }

    if (strcmp(_schema, "https") == 0) {
      out_port = 443;
      out_is_https = 1;
    } else if (strcmp(_schema, "http") == 0) {
      out_port = 80;
      out_is_https = 0;
    } else {
      PLOG_DEBUG(
          "http_parser_parse_url(%s) failed, schema only support http/https",
          raw_url);
      return NULL;
    }
  }

  if (url.field_set & (1 << UF_PORT)) {
    char _port[30] = {0};
    if (url.field_data[UF_PORT].len >= sizeof(_port) - 1) {
      PLOG_DEBUG("http_parser_parse_url(%s) failed, port is invalidate",
                 raw_url);
      return NULL;
    }

    memcpy(_port, raw_url + url.field_data[UF_PORT].off,
           url.field_data[UF_PORT].len);
    out_port = atoi(_port);
  }

  char *tmp = strchr(raw_url + url.field_data[UF_HOST].off, '/');
  int len = sizeof(http_client_url_info_t) + strlen(raw_url) + 1;
  if (client->host != NULL) {
    len += strlen(client->host) + 1;
  }

  /*
  request->host = ((char *)request) + sizeof(rc_http_request_t);
  request->path = request->host + url.field_data[UF_HOST].len + 1;

  memcpy(request->host, raw_url + url.field_data[UF_HOST].off,
         url.field_data[UF_HOST].len);
  if (tmp != NULL) {
    memcpy(request->path, tmp, strlen(tmp));
  } else {
    request->path[0] = '/';
  }*/
  return NULL;
}

void httpclient_on_connected(uv_connect_t *req, int status) {
  PLOG_DEBUG("connect status=%d", status);
  proton_connect_t *pc = (proton_connect_t *)req->data;
  pc->status = status;

  proton_coroutine_wakeup(UV_HANDLE_RUNTIME(req->handle), &pc->wq_connect,
                          NULL);
}

int proton_httpclient_connect(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_client_t *client = (proton_http_client_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  proton_connect_t pc = {
      .status = 0,
  };
  PROTON_WAIT_OBJECT_INIT(pc.wq_connect);
  pc.connect.data = &pc;

  struct sockaddr_in addr;
  int rc = uv_ip4_addr(client->host, client->port, &addr);
  if (rc != 0) {
    PLOG_WARN("parse ip(%s) failed.", client->host);
    return -1;
  }

  rc = uv_tcp_connect(&pc.connect, &client->connect->tcp,
                      (struct sockaddr *)&addr, httpclient_on_connected);
  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pc.wq_connect, NULL);
    if (pc.status != 0) {
      return pc.status;
    }
  }

  return rc;
}

extern int http_message_build_request_headers(proton_link_buffer_t *plb,
                                              proton_http_message_t *message,
                                              enum http_method method,
                                              const char *headers[],
                                              int header_count,
                                              int body_length);

int proton_httpclient_request(proton_private_value_t *value,
                              enum http_method method, const char *url,
                              const char *headers[], int header_count,
                              const char *body, int body_len) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *client = (proton_http_connect_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  if (proton_httpclient_connect(value) != 0) {
    return -1;
  }

  proton_link_buffer_t lbf;
  proton_link_buffer_init(&lbf, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  http_message_build_request_headers(&lbf, client->current, method, headers,
                                     header_count, body_len);

  uv_buf_t rbufs[2] = {
      {.base = proton_link_buffer_get_ptr(&lbf, 0), .len = lbf.total_used_size},
      {.base = (char *)body, .len = body_len}};

  proton_write_t pw;
  pw.writer.data = &pw;
  PROTON_WAIT_OBJECT_INIT(pw.wq_write);

  int rc = uv_write(&pw.writer, (uv_stream_t *)&client->tcp, rbufs,
                    body == NULL ? 1 : 2, httpconnect_on_write);

  client->current->write_status = PROTON_HTTP_STATUS_WRITE_DONE;

  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pw.wq_write, NULL);

    if (pw.writer.error != 0) {
      rc = pw.writer.error;
    }
  }

  proton_link_buffer_uninit(&lbf);

  return rc;
}