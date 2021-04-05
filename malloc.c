/*
    module  : malloc.c
    version : 1.2
    date    : 04/05/21
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
    cur->size = size;
    my_free(cur + 1);
    return 1;
}

void *my_malloc(long size)
{
    heap_t *cur, *prev;

    if (!size)
	return 0;
    size = 1 + (size + sizeof(heap_t) - 1) / sizeof(heap_t);
    do
	for (prev = 0, cur = my_free_list; cur; prev = cur, cur = cur->next)
	    if (cur->size >= size) {
		if (cur->size <= size + 1) {
		    if (prev)
			prev->next = cur->next;		/* skip one */
		    else
			my_free_list = cur->next;	/* skip start */
		} else {
		    cur->size -= size;			/* split */
		    cur += cur->size;			
		    cur->size = size;			/* upper part */
		}
		return cur + 1;
	    }
    while (morecore(size));
    return 0;
}
