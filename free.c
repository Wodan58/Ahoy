/*
    module  : free.c
    version : 1.2
    date    : 04/05/21
*/
#define EXTERN
#include "my_malloc.h"

void my_free(void *tmp)
{
    heap_t *cur, *ptr = tmp;

    if (!ptr--)					/* null pointer */
	return;
    if (!my_free_list) {			/* empty free list */
	my_free_list = ptr;
	ptr->next = 0;				/* initialize free list */
	return;
    }
    for (cur = my_free_list; cur->next && cur->next < ptr; cur = cur->next)
	;
    if (ptr + ptr->size == cur->next) {
	ptr->size += cur->next->size;
	ptr->next = cur->next->next;		/* swallow upper */
    } else
	ptr->next = cur->next;			/* connect to upper */
    if (cur + cur->size == ptr) {
	cur->size += ptr->size;
	cur->next = ptr->next;			/* swallow pointer */
    } else
	cur->next = ptr;			/* insert pointer */
}
