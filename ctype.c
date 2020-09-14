/*
    module  : ctype.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_libc.h"

long my_isspace(long ch)
{
    return ch < '!';
}

long my_isdigit(long ch)
{
    return ch >= '0' && ch <= '9';
}

long my_isupper(long ch)
{
    return ch >= 'A' && ch <= 'Z';
}

long my_islower(long ch)
{
    return ch >= 'a' && ch <= 'z';
}

long my_isalnum(long ch)
{
    return my_isdigit(ch) || my_isupper(ch) || my_islower(ch);
}
