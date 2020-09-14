/*
    module  : malloc.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_malloc.h"

#define NALLOC		256		/* #units to allocate at once */

static long morecore(long size)
{
    static long my_end;
    heap_t *cur;

    if (!my_end)
	my_end = my_break(0);
    cur = (heap_t *)my_end;
    size = NALLOC * ((size + NALLOC - 1) / NALLOC);
    my_end += size * sizeof(heap_t);
    if (my_break(my_end) == -1)
	return 0;			/* no space at all */
    cur->size = size - 1;
    my_free(cur + 1);
    return 1;
}

void *my_malloc(long size)
{
    heap_t *cur, *prev = 0, *next;

    if (!size)
	return 0;
    size = (size + sizeof(heap_t) - 1) / sizeof(heap_t);
    do
	for (cur = my_free_list; cur; prev = cur, cur = cur->next)
	    if (cur->size >= size + 2) {
		next = cur + cur->size - size;
		cur->size -= size + 1;
		next->size = size;
		return next + 1;
	    } else if (cur->size >= size) {
		if (prev)
		    prev->next = cur->next;
		else
		    my_free_list = my_free_list->next;
		return cur + 1;
	    }
    while (morecore(size));
    return 0;
}
