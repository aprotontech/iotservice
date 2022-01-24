/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     uvobject.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-16 10:14:27
 *
 */

#ifndef _PROTON_UVOBJECT_H_
#define _PROTON_UVOBJECT_H_

#include "runtime.h"
#include "proton/include/electron.h"
#include <uv.h>

#define MAKE_SURE_ON_COROTINUE(func)                                           \
  if (quark_coroutine_current() == quark_coroutine_get_main()) {               \
    QUARK_LOGGER("%s only can run on coroutine", func);                        \
    return -1;                                                                 \
  }

typedef struct _proton_buffer_t {
  list_link_t link;
  uv_buf_t buff;
  int used;
} proton_buffer_t;

typedef struct _proton_link_buffer_t {
  list_link_t link;
  size_t slice_size;
  size_t total_alloc_size;
  size_t total_used_size;
  size_t max_alloc_size;
  size_t max_used_size;
} proton_link_buffer_t;

typedef struct _proton_tcpserver_t {
  proton_private_value_t value;
  uv_tcp_t tcp;
  int new_connection_count;
  list_link_t waiting_coroutines;
  quark_coroutine_task *close_task;
} proton_tcpserver_t;

typedef struct _proton_read_context_t {
  uv_buf_t buffer;
  ssize_t filled;

  quark_coroutine_task *coroutine;
} proton_read_context_t;

typedef struct _proton_tcpclient_t {
  proton_private_value_t value;
  uv_tcp_t tcp;
  proton_read_context_t *reading;
  quark_coroutine_task *close_task;
} proton_tcpclient_t;

//////////// TCP-SERVER
proton_private_value_t *
proton_tcpserver_create(quark_coroutine_runtime *runtime);

int proton_tcpserver_listen(proton_private_value_t *value, const char *host,
                            int port);
int proton_tcpserver_accept(proton_private_value_t *value,
                            proton_private_value_t **client);

int proton_tcpserver_close(proton_private_value_t *value);

int proton_tcpserver_free(proton_private_value_t *value);

//////////// TCP-CLIENT
proton_private_value_t *
proton_tcpclient_create(quark_coroutine_runtime *runtime);

int proton_tcpclient_connect(proton_private_value_t *value, const char *host,
                             int port);

int proton_tcpclient_write(proton_private_value_t *value, const char *data,
                           int len);

int proton_tcpclient_read(proton_private_value_t *value, char *data, int len);

int proton_tcpclient_close(proton_private_value_t *value);

int proton_tcpclient_free(proton_private_value_t *value);

///// proton buffer link
proton_link_buffer_t *proton_link_buffer_init(proton_link_buffer_t *lbf,
                                              size_t slice_size,
                                              size_t max_size);

char *proton_link_buffer_alloc(proton_link_buffer_t *lbf, size_t len,
                               size_t align_size);

char *proton_link_buffer_copy_string(proton_link_buffer_t *lbf, const char *ptr,
                                     size_t len);

char *proton_link_buffer_append_string(proton_link_buffer_t *lbf,
                                       const char *ptr, size_t len);

proton_buffer_t *proton_link_buffer_new_slice(proton_link_buffer_t *lbf,
                                              size_t length);

char *proton_link_buffer_get_ptr(proton_link_buffer_t *lbf, size_t offset);

int proton_link_buffer_uninit(proton_link_buffer_t *lbf);

#endif