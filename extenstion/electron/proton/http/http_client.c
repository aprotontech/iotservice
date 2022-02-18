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
#include "php_request.h"

PROTON_TYPE_WHOAMI_DEFINE(_http_client_get_type, "httpclient")

static proton_value_type_t __proton_httpclient_type = {
    .construct = NULL,
    .destruct = proton_httpclient_uninit,
    .whoami = _http_client_get_type};

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

int http_client_response_handler(proton_private_value_t *self,
                                 proton_http_connect_t *connection,
                                 proton_http_message_t *message) {
  php_http_request_t *request =
      container_of(message, php_http_request_t, message);

  // if some one is wait parse done, wakeup them
  if (!LL_isspin(&request->wq_parser.head)) {
    proton_coroutine_wakeup(request->runtime, &request->wq_parser, NULL);
  }

  return 0;
}

proton_private_value_t *
proton_httpclient_create(proton_coroutine_runtime *runtime, const char *host,
                         int port) {
  int host_len = host != NULL ? strlen(host) : 0;
  proton_http_client_t *client = (proton_http_client_t *)qmalloc(
      sizeof(proton_http_client_t) + host_len + 1);

  proton_http_connect_t *connect =
      (proton_http_connect_t *)qmalloc(sizeof(proton_http_connect_t));

  client->connect = connect;
  client->runtime = runtime;
  client->value.type = &__proton_httpclient_type;
  ZVAL_UNDEF(&client->value.myself);
  client->host = NULL;
  client->port = port;
  if (host != NULL) {
    client->host = (char *)(&client[1]);
    strcpy(client->host, host);
  }
  client->callbacks.self = &client->value;
  client->callbacks.request_handler = http_client_response_handler;

  if (proton_http_connection_init(connect, runtime, &client->callbacks,
                                  HTTP_RESPONSE) != 0) {
    PLOG_WARN("init http connect failed");
    qfree(client);
    qfree(connect);
    return NULL;
  }

  return &client->value;
}

int proton_httpclient_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_client_t *client = (proton_http_client_t *)value;
  if (client->connect != NULL &&
      Z_TYPE_P(&client->connect->value.myself) == IS_OBJECT) {
    RELEASE_VALUE_MYSELF(client->connect->value);
    client->connect = NULL;
  }
  return 0;
}

http_client_url_info_t *http_client_parse_url(proton_http_client_t *client,
                                              const char *raw_url) {
  size_t raw_url_length = strlen(raw_url);
  struct http_parser_url url;
  int ret = http_parser_parse_url(raw_url, raw_url_length, 0, &url);
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
  int out_port = client->port != 0 ? client->port : 80;

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

  int len =
      sizeof(http_client_url_info_t) + raw_url_length + 2 + sizeof("127.0.0.1");
  if (client->host != NULL) {
    len += strlen(client->host) + 1;
  }

  char *ptr = (char *)qmalloc(len);
  http_client_url_info_t *info = (http_client_url_info_t *)ptr;
  info->port = out_port;
  info->is_https = out_is_https;
  info->path = ptr + sizeof(http_client_url_info_t);

  // COPY PATH CONTENT
  char *path_ptr = strchr(raw_url + url.field_data[UF_HOST].off, '/');
  if (path_ptr != NULL) {
    size_t path_length = raw_url_length - (path_ptr - raw_url);
    memcpy(info->path, path_ptr, path_length);
    info->path[path_length] = '\0';

    info->host = info->path + path_length + 1;
  } else {
    info->path[0] = '/';
    info->path[1] = '\0';
    info->host = info->path + 2;
  }

  // COPY HOST CONTENT
  if (url.field_set & (1 << UF_HOST)) { // use host here
    memcpy(info->host, raw_url + url.field_data[UF_HOST].off,
           url.field_data[UF_HOST].len);
    info->host[url.field_data[UF_HOST].len] = '\0';
  } else if (client->host != NULL) {
    strcpy(info->host, client->host);
  } else {
    strcpy(info->host, "127.0.0.1");
  }

  return info;
}

extern int http_message_build_request_headers(
    proton_link_buffer_t *plb, proton_http_message_t *message,
    enum http_method method, const char *host, const char *headers[],
    int header_count, int body_length);

int proton_httpclient_request(proton_private_value_t *value,
                              enum http_method method, const char *url,
                              const char *headers[], int header_count,
                              const char *body, int body_len) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(url);
  proton_http_client_t *client = (proton_http_client_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  if (method == HTTP_GET || method == HTTP_DELETE) {
    // GET/DELETE request skip body
    body = NULL;
    body_len = 0;
  }

  http_client_url_info_t *info = http_client_parse_url(client, url);
  if (info == NULL) {
    PLOG_WARN("parse url(%s) failed", url);
    return -1;
  }

  PLOG_INFO("url(%s)-->https(%d),host(%s),port(%d),path(%s)", url,
            info->is_https, info->host, info->port, info->path);

  if (proton_httpconnect_connect(
          client->connect, client->host != NULL ? client->host : info->host,
          client->port != 0 ? client->port : info->port) != 0) {
    return -1;
  }

  if (httpconnect_start_message(client->connect) != 0) {
    return -1;
  }

  proton_link_buffer_t lbf;
  proton_link_buffer_init(&lbf, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  client->connect->current->path = info->path;
  http_message_build_request_headers(&lbf, client->connect->current, method,
                                     info->host, headers, header_count,
                                     body_len);
  uv_buf_t rbufs[2] = {
      {.base = proton_link_buffer_get_ptr(&lbf, 0), .len = lbf.total_used_size},
      {.base = (char *)body, .len = body_len}};

  int rc = httpconnect_write(client->connect, rbufs, body == NULL ? 1 : 2);
  if (rc != 0) {
    PLOG_WARN("write data failed, with %d", rc);
  }

  proton_link_buffer_uninit(&lbf);

  return rc;
}