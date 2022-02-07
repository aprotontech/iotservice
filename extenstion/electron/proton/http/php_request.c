/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     php_request.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-06 10:13:52
 *
 */

#include "php_request.h"

PROTON_TYPE_WHOAMI_DEFINE(_php_request_get_type, "httprequest")

int php_http_request_uninit(proton_private_value_t *value);
static proton_value_type_t __proton_http_request_type = {
    .construct = NULL,
    .destruct = php_http_request_uninit,
    .whoami = _php_request_get_type};

proton_private_value_t *php_request_create(proton_http_connect_t *client) {
  if (client == NULL) {
    PLOG_WARN("input client is null");
    return NULL;
  }
  php_http_request_t *request =
      (php_http_request_t *)qmalloc(sizeof(php_http_request_t));
  request->value.type = &__proton_http_request_type;
  request->connect = client;
  ZVAL_UNDEF(&request->value.myself);
  Z_TRY_ADDREF(client->value.myself);
  return &request->value;
}

int php_http_request_uninit(proton_private_value_t *value) {
  php_http_request_t *request = (php_http_request_t *)value;
  MAKESURE_PTR_NOT_NULL(value);

  // release client ref-count
  RELEASE_VALUE_MYSELF(request->connect->value);

  request->connect = NULL;
  return 0;
}

extern void _httpconnect_restart_parse(proton_http_connect_t *client);
extern int http_message_build_response_headers(
    proton_link_buffer_t *plb, proton_http_message_t *message, int status_code,
    const char *headers[], int header_count, int body_length);

int proton_httpconnect_write_response(proton_private_value_t *value,
                                      int status_code, const char *headers[],
                                      int header_count, const char *body,
                                      int body_len) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *client = (proton_http_connect_t *)value;
  MAKESURE_ON_COROTINUE(client->runtime);

  proton_link_buffer_t lbf;
  proton_link_buffer_init(&lbf, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  http_message_build_response_headers(&lbf, client->current, status_code,
                                      headers, header_count, body_len);

  uv_buf_t rbufs[2] = {
      {.base = proton_link_buffer_get_ptr(&lbf, 0), .len = lbf.total_used_size},
      {.base = (char *)body, .len = body_len}};

  proton_write_t pw;
  pw.writer.data = &pw;
  PROTON_WAIT_OBJECT_INIT(pw.wq_write);

  int rc = uv_write(&pw.writer, (uv_stream_t *)&client->tcp, rbufs, 2,
                    httpconnect_on_write);

  client->current->write_status = PROTON_HTTP_STATUS_WRITE_DONE;

  if (rc == 0) {
    proton_coroutine_waitfor(client->runtime, &pw.wq_write, NULL);

    if (pw.writer.error != 0) {
      rc = pw.writer.error;
    }
  }

  proton_link_buffer_uninit(&lbf);

  if (client->current->keepalive) {
    // all response had send, so clean current context
    // wait for next request
    _httpconnect_restart_parse(client);
  }

  return rc;
}