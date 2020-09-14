/*
    module  : my_malloc.h
    version : 1.1
    date    : 09/12/20
*/
#ifndef _MALLOC_H
#define _MALLOC_H

#include "my_libc.h"

typedef struct heap_t {
    long size;
    struct heap_t *next;
} heap_t;

EXTERN heap_t *my_free_list;
#endif
