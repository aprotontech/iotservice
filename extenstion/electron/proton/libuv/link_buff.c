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
#include "link_buff.h"

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

int proton_link_buffer_append_slice(proton_link_buffer_t *lbf,
                                    proton_buffer_t *buffer) {
  MAKESURE_PTR_NOT_NULL(lbf);
  MAKESURE_PTR_NOT_NULL(buffer);

  LL_insert(&buffer->link, lbf->link.prev);
  lbf->total_alloc_size += buffer->buff.len;
  lbf->total_used_size += buffer->used;

  return 0;
}

proton_buffer_t *proton_link_buffer_new_slice(proton_link_buffer_t *lbf,
                                              size_t length) {
  length = ((length + lbf->slice_size - 1) / lbf->slice_size) * lbf->slice_size;
  if (lbf->total_alloc_size + length >= lbf->max_alloc_size) {
    PLOG_INFO("total alloc size max than %d", (int)lbf->max_alloc_size);
    return NULL;
  }

  proton_buffer_t *ptr = (proton_buffer_t *)qmalloc(length);
  ptr->buff.base = (char *)&ptr[1];
  ptr->buff.len = length - sizeof(proton_buffer_t);
  ptr->used = 0;
  ptr->need_free = 1;
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
  dst[len] = '\0';
  pbt->used += len + 1;
  return dst;
}

char *proton_link_buffer_append_string(proton_link_buffer_t *lbf,
                                       const char *ptr, size_t len) {
  char *dest = proton_link_buffer_alloc(lbf, len, 1);
  if (dest != NULL) {
    memcpy(dest, ptr, len);
    // dest[len] = '\0';
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

zend_string *proton_link_to_string(proton_link_buffer_t *lbf) {
  if (lbf == NULL) {
    return NULL;
  }

  zend_string *s = zend_string_alloc(lbf->total_used_size, 0);
  if (s != NULL) {
    ZSTR_VAL(s)[lbf->total_used_size] = '\0';
    list_link_t *p = lbf->link.next;
    char *q = ZSTR_VAL(s);
    while (p != &lbf->link) {
      proton_buffer_t *pbt = container_of(p, proton_buffer_t, link);
      p = p->next;

      memcpy(q, pbt->buff.base, pbt->used);

      q += pbt->used;
    }
  }

  return s;
}

int proton_link_buffer_uninit(proton_link_buffer_t *lbf) {
  MAKESURE_PTR_NOT_NULL(lbf);
  list_link_t *p = lbf->link.next;
  while (p != &lbf->link) {
    proton_buffer_t *pbt = container_of(p, proton_buffer_t, link);
    p = LL_remove(p);

    if (pbt->need_free) {
      qfree(pbt);
    }
  }

  return 0;
}