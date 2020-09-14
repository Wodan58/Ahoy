/*
    module  : free.c
    version : 1.1
    date    : 09/12/20
*/
#define EXTERN
#include "my_malloc.h"

void my_free(void *ptr)
{
    heap_t *cur, *prev = 0, *next = ptr;

    if (!next--)
	return;
    for (cur = my_free_list; cur && cur < next; prev = cur, cur = cur->next)
	;
    if (next + next->size + 1 == cur) {
	next->size += cur->size + 1;
	next->next = cur->next;
    } else
	next->next = cur;
    if (!prev)
	my_free_list = next;
    else if (prev + prev->size + 1 == next) {
	prev->size += next->size + 1;
	prev->next = next->next;
    } else
	prev->next = next;
}
