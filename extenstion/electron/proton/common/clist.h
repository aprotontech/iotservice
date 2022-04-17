/***************************************************************************
 *
 * Copyright (c) 2019 aproton.tech, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @file     clist.h
 * @author   kuper - kuper@aproton.tech
 * @data     2019-12-13 12:10:47
 * @version  0
 * @brief
 *
 **/

#ifndef _QUARK_CLIST_H_
#define _QUARK_CLIST_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct _rc_list_link_t {
  struct _rc_list_link_t *prev;
  struct _rc_list_link_t *next;
} list_link_t;

list_link_t *LL_init(list_link_t *node);
list_link_t *LL_insert(list_link_t *insert_node, list_link_t *after_node);
list_link_t *LL_remove(list_link_t *node);
list_link_t *LL_swap(list_link_t *n1, list_link_t *n2);
int LL_isspin(list_link_t *node);
int LL_size(list_link_t *node);

#endif
