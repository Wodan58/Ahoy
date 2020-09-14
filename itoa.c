/*
    module  : itoa.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_libc.h"

static void writenat(unsigned long num, char **ptr)
{
    if (num >= 10)
	writenat(num / 10, ptr);
    **ptr = num % 10 + '0';
    (*ptr)++;
}

/*
    itoa converts an integer to an ASCII string.
*/
char *itoa(long num, char *ptr)
{
    if (num < 0) {
	num = -num;
	*ptr++ = '-';
    }
    writenat(num, &ptr);
    *ptr = 0;
    return ptr;
}
