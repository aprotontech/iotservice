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

proton_private_value_t *php_request_create(proton_http_client_t *client) {
  if (client == NULL) {
    PLOG_WARN("input client is null");
    return NULL;
  }
  php_http_request_t *request =
      (php_http_request_t *)qmalloc(sizeof(php_http_request_t));
  request->value.type = &__proton_http_request_type;
  request->client = client;
  request->message = &client->current;
  request->message_version = client->message_version;
  ZVAL_UNDEF(&request->value.myself);
  Z_TRY_ADDREF(client->value.myself);
  return &request->value;
}

int php_http_request_uninit(proton_private_value_t *value) {
  php_http_request_t *request = (php_http_request_t *)value;
  MAKESURE_PTR_NOT_NULL(value);

  // release client ref-count
  RELEASE_VALUE_MYSELF(request->client->value);

  request->client = NULL;
  request->message = NULL;
  return 0;
}