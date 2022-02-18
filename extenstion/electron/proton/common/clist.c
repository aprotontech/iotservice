/***************************************************************************
 *
 * Copyright (c) 2019 aproton.tech, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @file     clist.c
 * @author   kuper - kuper@aproton.tech
 * @data     2019-12-13 18:47:09
 * @version  0
 * @brief
 *
 **/

#include "clist.h"

list_link_t* LL_init(list_link_t* node)
{
    node->prev = node;
    node->next = node;
    return node;
}

list_link_t* LL_insert(list_link_t* insert_node, list_link_t* after_node)
{
    if (insert_node == NULL) return after_node;
    else if (after_node == NULL) return insert_node;
    else if (insert_node == after_node) return insert_node;

    insert_node->prev = after_node;
    insert_node->next = after_node->next;
    after_node->next = insert_node;
    if (insert_node->next != NULL) {
        insert_node->next->prev = insert_node;
    }
    return insert_node;
}

list_link_t* LL_remove(list_link_t* node)
{
    list_link_t* prev = node->prev;
    list_link_t* next = node->next;
    if (prev != NULL) {
        prev->next = node->next;
    }
    if (next != NULL) {
        next->prev = node->prev;
    }
    node->prev = node;
    node->next = node;
    return next;
}

list_link_t* LL_swap(list_link_t* n1, list_link_t* n2)
{
    if (n1 == n2) return n1;

    return NULL;
}

int LL_isspin(list_link_t* node)
{
    return node != NULL &
        node->next == node && node->prev == node;
}

