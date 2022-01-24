/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     link_buff.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-01-21 12:15:40
 *
 */

#include "uvobject.h"

proton_link_buffer_t *proton_link_buffer_init(proton_link_buffer_t *lbf,
                                              size_t slice_size,
                                              size_t max_size) {
  assert(lbf != NULL);
  LL_init(&lbf->link);
  lbf->max_alloc_size = max_size;
  lbf->max_used_size = max_size;
  lbf->slice_size = slice_size;
  lbf->total_alloc_size = 0;
  lbf->total_used_size = 0;
}

proton_buffer_t *proton_link_buffer_new_slice(proton_link_buffer_t *lbf,
                                              size_t length) {
  length = ((length + lbf->slice_size - 1) / lbf->slice_size) * lbf->slice_size;
  if (lbf->total_alloc_size + length >= lbf->max_alloc_size) {
    QUARK_LOGGER("total alloc size max than %d", (int)lbf->max_alloc_size);
    return NULL;
  }

  proton_buffer_t *ptr = (proton_buffer_t *)qmalloc(length);
  ptr->buff.base = (char *)&ptr[1];
  ptr->buff.len = length - sizeof(proton_buffer_t);
  ptr->used = 0;
  LL_insert(&ptr->link, lbf->link.prev);
  lbf->total_alloc_size += length;
  return ptr;
}

char *proton_link_buffer_alloc(proton_link_buffer_t *lbf, size_t len,
                               size_t align_size) {
  assert(lbf != NULL);
  proton_buffer_t *pbt = container_of(lbf->link.prev, proton_buffer_t, link);
  char *ptr = NULL;
  size_t used = ((pbt->used + align_size - 1) / align_size) * align_size;
  if (used + len > pbt->buff.len) {
    pbt = proton_link_buffer_new_slice(lbf, len);
    if (pbt == NULL) {
      return NULL;
    } else {
      ptr = pbt->buff.base;
      pbt->used = len;
      lbf->total_used_size += len;
    }
  } else {
    lbf->total_used_size += (used - pbt->used) + len;
    ptr = pbt->buff.base + used;
    pbt->used = used + len;
  }

  return ptr;
}

char *proton_link_buffer_copy_string(proton_link_buffer_t *lbf, const char *ptr,
                                     size_t len) {
  proton_buffer_t *pbt = container_of(lbf->link.prev, proton_buffer_t, link);
  if (pbt->used + len + 1 >= pbt->buff.len) {
    pbt = proton_link_buffer_new_slice(lbf, len + 1);
    if (pbt == NULL) {
      return NULL;
    }
  }

  char *dst = pbt->buff.base + pbt->used;
  memcpy(dst, ptr, len);
  dst[len] = 0;
  pbt->used += len + 1;
  return dst;
}

char *proton_link_buffer_append_string(proton_link_buffer_t *lbf,
                                       const char *ptr, size_t len) {
  char *dest = proton_link_buffer_alloc(lbf, len, 1);
  if (dest != NULL) {
    memcpy(dest, ptr, len);
  }
  return dest;
}

char *proton_link_buffer_get_ptr(proton_link_buffer_t *lbf, size_t offset) {
  list_link_t *p = lbf->link.next;
  while (p != &lbf->link) {
    proton_buffer_t *pbt = container_of(p, proton_buffer_t, link);
    if (offset < pbt->used) {
      return pbt->buff.base + offset;
    }

    offset -= pbt->used;
    p = p->next;
  }

  return 0;
}

int proton_link_buffer_uninit(proton_link_buffer_t *lbf) { return 0; }