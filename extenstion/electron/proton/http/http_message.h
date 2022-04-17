/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_message.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-12 10:07:40
 *
 */

#ifndef _PROTON_HTTP_MESSAGE_H_
#define _PROTON_HTTP_MESSAGE_H_

#include "proton/common/electron.h"
#include "proton/libuv/link_buff.h"

typedef enum _proton_http_content_type {
  PROTON_HTTP_CT_NORMAL = 0,
  PROTON_HTTP_CT_MULTIPART = 1,
  PROTON_HTTP_CT_FORM = 2,

} proton_http_content_type;

typedef struct _proton_header_t {
  list_link_t link; // more than one header has same key
  uv_buf_t key;
  uv_buf_t value;
} proton_header_t;

typedef struct _proton_multipart_t {
  list_link_t link;

  uv_buf_t name;
  uv_buf_t filename;
  uv_buf_t content;
  uv_buf_t encode;
  uv_buf_t type;

  int is_file;

  // FILE INFO
  uv_buf_t tmp_name;
  int error;
} proton_multipart_t;

typedef enum _proton_http_message_status {
  // write status
  PROTON_HTTP_STATUS_WRITE_NONE = 0,
  PROTON_HTTP_STATUS_WRITE_HEAD = 1,
  PROTON_HTTP_STATUS_WRITE_BODY = 2,
  PROTON_HTTP_STATUS_WRITE_DONE = 3,

  // read status
  PROTON_HTTP_STATUS_READ_NONE = 0,
  PROTON_HTTP_STATUS_READ_HEAD = 1,
  PROTON_HTTP_STATUS_READ_BODY = 2,
  PROTON_HTTP_STATUS_READ_DONE = 3,
} proton_http_message_status;

// typedef struct _proton_http_connect_t proton_http_connect_t;
typedef struct _proton_http_message_t {

  http_parser parser;
  http_parser_settings settings;
  proton_header_t *current_header;
  char keepalive;
  char parse_finished;

  enum http_method method;
  uv_buf_t path;

  proton_link_buffer_t buffers; // buffers

  proton_http_message_status read_status;
  proton_http_message_status write_status;

  ///////////// REQUEST/RESPONSE
  char is_chunk_mode;
  list_link_t headers;
  proton_http_content_type content_type;
  proton_link_buffer_t body; // response body

  zend_string *raw_body;

  //////////// MULTIPART
  // item: proton_multipart_t
  list_link_t multiparts;
  uv_buf_t boundary;
  int is_parsed_multipart;

} proton_http_message_t;

typedef struct _proton_http_connect_t proton_http_connect_t;

int http_message_init(proton_http_connect_t *client,
                      proton_http_message_t *message,
                      enum http_parser_type type);
int http_message_uninit(proton_http_message_t *request);

int http_message_get_multipart_content(proton_http_message_t *message,
                                       zval *_post, zval *_file);

zend_string *http_message_get_raw_body(proton_http_message_t *message);

#endif