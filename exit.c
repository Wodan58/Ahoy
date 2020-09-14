/*
    module  : exit.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_libc.h"

#define MAXTAB		10

static long index;
static proc_t table[MAXTAB];

void my_atexit(proc_t proc)
{
    if (index == MAXTAB)
	return;
    table[index++] = proc;
}

void my_exit(long num)
{
    while (--index >= 0)
	(*table[index])();
    _exit(num);
}
