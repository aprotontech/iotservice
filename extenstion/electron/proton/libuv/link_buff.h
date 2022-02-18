/*
 * **************************************************************************
 * 
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 * 
 * **************************************************************************
 * 
 *  @file     link_buff.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-31 07:04:44
 * 
 */

#ifndef _PROTON_LINK_BUFF_H_
#define _PROTON_LINK_BUFF_H_

#include "proton/common/electron.h"

#include <uv.h>


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