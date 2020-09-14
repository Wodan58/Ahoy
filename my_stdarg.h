/*
    module  : my_stdarg.h
    version : 1.1
    date    : 09/12/20
*/
#ifndef _STDARG_H_
#define _STDARG_H_

typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#define va_end(v)	__builtin_va_end(v)
#endif
