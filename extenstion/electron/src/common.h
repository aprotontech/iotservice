/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     electron.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-18 11:43:43
 *
 */

#ifndef _QUARK_ELECTRON_H_
#define _QUARK_ELECTRON_H_

#include "proton/common/electron.h"
#include "php_electron.h"
#include "zend_exceptions.h"

#include "proton/libuv/uvobject.h"
#include "proton/coroutine/runtime.h"
#include "proton/coroutine/channel.h"
#include "proton/http/http.h"
#include "proton/http/php_request.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define PROTON_NAMESPACE "Proton"

#define PHP_PRIVATE_VALUE_RESOURCE_NAME "proton_private_value"

#define PROTON_OBJECT_PRIVATE_VALUE "_proton_private"
#define PROTON_HTTPSERVER_DEFAULT_ROUTER_VALUE "_default_router"

extern int pc_private_resource_handle;
extern proton_uv_scheduler *__uv_scheduler;

extern proton_private_value_t *proton_object_get(zval *self);
extern int proton_object_construct(zval *self, proton_private_value_t *val);
extern int proton_object_destruct(zval *self);
extern void destruct_proton_private_value(zend_resource *rsrc);
extern proton_coroutine_runtime *proton_get_runtime();

const char **get_input_headers(zval *headers, int *output_headers_count);

extern zend_class_entry *_channel_ce;
extern zend_class_entry *_tcpclient_ce;
extern zend_class_entry *_tcpserver_ce;
extern zend_class_entry *_runtime_ce;
extern zend_class_entry *_coroutine_ce;
extern zend_class_entry *_httpserver_ce;
extern zend_class_entry *_httpconnect_ce;
extern zend_class_entry *_httpclient_ce;
extern zend_class_entry *_httprequest_ce;
extern zend_class_entry *_httpresponse_ce;
extern zend_class_entry *_processgroup_ce;

#endif