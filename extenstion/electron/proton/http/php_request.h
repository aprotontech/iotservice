/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     php_request.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-06 10:13:08
 *
 */

#ifndef _PROTON_PHP_REQUEST_H_
#define _PROTON_PHP_REQUEST_H_

#include "http.h"

// PHP HTTP-REQUEST-OBJECT
typedef struct _php_http_request_t {
  proton_private_value_t value;
  proton_coroutine_runtime *runtime;

  proton_http_connect_t *connect;

  proton_http_message_t message;

  proton_wait_object_t wq_parser;

} php_http_request_t;

int php_request_init(proton_http_connect_t *connect,
                     php_http_request_t *request);
int php_request_uninit(proton_private_value_t *request);

int php_request_wait_parse_finish(php_http_request_t *request);

#endif