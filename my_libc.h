/*
    module  : my_libc.h
    version : 1.1
    date    : 09/12/20
*/
#ifndef _LIBC_H
#define _LIBC_H

#ifndef EXTERN
#define EXTERN		extern
#endif

#define OK		0
#define ERROR		-1

#define STDIN		0
#define STDOUT		1
#define STDERR		2

#define O_RDONLY	    0
#define O_WRONLY	    1
#define O_RDWR		    2
#define O_CREAT		 0100
#define O_EXCL		 0200
#define O_NOCTTY	 0400
#define O_TRUNC		01000
#define O_APPEND	02000
#define O_NONBLOCK	04000

typedef long clock_t;

typedef void (*proc_t)(void);

long my_fgetc(long fd);
long my_fgets(char *str, long size, long fd);

void my_flush(long fd);
void my_fputc(long ch, long fd);
void my_fprint(long fd, char *format, ...);
void my_fputs(char *str, long fd);

char *itoa(long num, char *buf);

void my_memset(char *str, long ch, long leng);
void my_strcpy(char *dst, char *src);
void my_strncpy(char *dst, char *src, long leng);
long my_strlen(char *str);
long my_strcmp(char *dst, char *src);
long my_strchr(char *str, long ch);
char *my_strdup(char *str);

void my_atexit(proc_t proc);
void my_exit(long num);

long my_isspace(long ch);
long my_isdigit(long ch);
long my_isupper(long ch);
long my_islower(long ch);
long my_isalnum(long ch);

void *my_malloc(long size);
void my_free(void *ptr);

long my_setjmp(long *buf);
void my_longjmp(long *buf, long rv);

void _exit(long num);
long my_read(long fd, char *buf, long size);
long my_write(long fd, char *buf, long size);
long my_open(char *name, long mode, long perm);
void my_close(long fd);
long my_break(long num);
clock_t my_clock(void);
#endif
