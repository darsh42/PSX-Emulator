#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include "common.h"

typedef struct LINKED_LIST_NODE {
    uint32_t value;
    struct LINKED_LIST_NODE *next;
} ll_node_t;

#endif // LINKEDLIST_H_INCLUDED
