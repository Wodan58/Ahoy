/*
    module  : string.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_libc.h"

void my_memset(char *str, long ch, long leng)
{
    while (leng-- > 0)
	*str++ = ch;
}

void my_strcpy(char *dst, char *src)
{
    while ((*dst++ = *src++) != 0)
	;
}

void my_strncpy(char *dst, char *src, long leng)
{
    while (leng-- > 0)
	if ((*dst++ = *src++) == 0)
	    break;
}

long my_strlen(char *str)
{
    long leng = 0;

    while (*str++)
	leng++;
    return leng;
}

long my_strcmp(char *dst, char *src)
{
    while (*dst && *dst == *src) {
	dst++;
	src++;
    }
    return *dst - *src;
}

long my_strchr(char *str, long ch)
{
    while (*str)
	if (*str++ == ch)
	    return 1;
    return 0;
}

char *my_strdup(char *str)
{
    char *ptr;

    ptr = my_malloc(my_strlen(str) + 1);
    if (ptr)
	my_strcpy(ptr, str);
    return ptr;
}
