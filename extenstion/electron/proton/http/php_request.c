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

static proton_value_type_t __proton_http_request_type = {
    .construct = NULL,
    .destruct = php_request_uninit,
    .whoami = _php_request_get_type};

php_http_request_t *php_request_create(proton_http_connect_t *connect) {
  php_http_request_t *request =
      (php_http_request_t *)qmalloc(sizeof(php_http_request_t));
  if (php_request_init(connect, request) != 0) {
    qfree(request);
    return NULL;
  }

  return request;
}

int php_request_init(proton_http_connect_t *connect,
                     php_http_request_t *request) {
  MAKESURE_PTR_NOT_NULL(connect);
  MAKESURE_PTR_NOT_NULL(request);

  request->value.type = &__proton_http_request_type;
  request->connect = connect;
  request->runtime = connect->runtime;
  ZVAL_UNDEF(&request->value.myself);

  if (Z_TYPE_P(&connect->value.myself) == IS_OBJECT) {
    Z_TRY_ADDREF(connect->value.myself);
  }

  PROTON_WAIT_OBJECT_INIT(request->wq_parser);
  request->wq_parser.mode = QC_MODE_ALL;

  http_message_init(connect, &request->message, connect->type);

  return 0;
}

int php_request_uninit(proton_private_value_t *value) {
  php_http_request_t *request = (php_http_request_t *)value;
  MAKESURE_PTR_NOT_NULL(value);

  if (!LL_isspin(&request->wq_parser.head)) {
    proton_coroutine_wakeup(request->runtime, &request->wq_parser, NULL);
  }

  http_message_uninit(&request->message);

  // release client ref-count
  if (Z_TYPE_P(&request->connect->value.myself) == IS_OBJECT) {
    RELEASE_VALUE_MYSELF(request->connect->value);
  }

  request->connect = NULL;
  return 0;
}

extern int http_message_build_response_headers(
    proton_link_buffer_t *plb, proton_http_message_t *message, int status_code,
    const char *headers[], int header_count, int body_length);

int proton_httpconnect_write_response(proton_private_value_t *value,
                                      int status_code, const char *headers[],
                                      int header_count, const char *body,
                                      int body_len) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_http_connect_t *connect = (proton_http_connect_t *)value;
  MAKESURE_ON_COROTINUE(connect->runtime);

  proton_link_buffer_t lbf;
  proton_link_buffer_init(&lbf, HTTPCLIENT_DEFAULT_ALLOC_SIZE,
                          HTTPCLIENT_MAX_BUFFER_SIZE);

  http_message_build_response_headers(&lbf, connect->current, status_code,
                                      headers, header_count, body_len);

  int rc = httpconnect_write_raw_message(connect, &lbf, body, body_len);
  if (rc != 0) {
    PLOG_WARN("write data failed, with %d", rc);
  }

  proton_link_buffer_uninit(&lbf);

  // all response had send, so clean current context
  // wait for next request
  return httpconnect_finish_message(connect);
}

int php_request_wait_parse_finish(php_http_request_t *request) {
  MAKESURE_PTR_NOT_NULL(request);

  if (request->message.parse_finished) {
    return 0;
  }

  MAKESURE_ON_COROTINUE(request->runtime);
  return proton_coroutine_waitfor(request->runtime, &request->wq_parser, NULL);
}
