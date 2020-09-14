/*
    module  : print.c
    version : 1.1
    date    : 09/12/20
*/
#include "my_files.h"
#include "my_stdarg.h"

/*
    my_flush flushes my_buf to fd.
*/
void my_flush(long fd)
{
    if (cur_pos[fd])
	my_write(fd, my_buf[fd], cur_pos[fd]);
    cur_pos[fd] = 0;
}

/*
    my_fputc behaves like fputc.
*/
void my_fputc(long ch, long fd)
{
    if (cur_pos[fd] == MAXBUF)
	my_flush(fd);
    my_buf[fd][cur_pos[fd]++] = ch;
    if (ch == '\n')
	my_flush(fd);
}

void my_fprint(long fd, char *format, ...)
{
    static char buf[MAXSTR];
    va_list argv;
    char *bp;
    long ch, justify, width, prec, leng;

    va_start(argv, format);
    while ((ch = *format++) != 0) {
	if (ch != '%')
	    my_fputc(ch, fd);
	else {
	    bp = buf;
	    justify = width = prec = leng = 0;
	    if (*format == '-') {
		format++;
		justify = 1;
	    }
	    if (*format == '*') {
		format++;
		width = va_arg(argv, long);
		if (width < 0) {
		    width = -width;
		    justify = 1 - justify;
		}
	    } else
		while (my_isdigit(*format))
		    width = 10 * width + *format++ - '0';
	    if (*format == '.') {
		format++;
		if (*format == '*') {
		    format++;
		    prec = va_arg(argv, long);
		} else
		    while (my_isdigit(*format))
			prec = 10 * prec + *format++ - '0';
		if (prec < 0)
		    prec = 0;
	    }
again:
	    switch (*format) {
	    default : *bp++ = *format;
		      break;
	    case 'l': format++;
		      goto again;
	    case 'c': ch = va_arg(argv, long);
		      if (ch)
			  *bp++ = ch;
		      break;
	    case 's': bp = va_arg(argv, char *);
		      break;
	    case 'd':
	    case 'u': ch = va_arg(argv, long);
		      bp = itoa(ch, bp);
		      break;
	    }
	    if (*format++ == 's') {
		leng = my_strlen(bp);
		if (leng > prec && prec)
		    leng = prec;
	    } else {
		leng = bp - buf;
		*bp = 0;
		bp = buf;
	    }
	    if ((width -= leng) < 0)
		width = 0;
	    if (!justify)
		while (width-- > 0)
		    my_fputc(' ', fd);
	    while (leng-- > 0)
		my_fputc(*bp++, fd);
	    if (justify)
		while (width-- > 0)
		    my_fputc(' ', fd);
	}
    }
    va_end(argv);
}

void my_fputs(char *str, long fd)
{
    while (*str)
	my_fputc(*str++, fd);
}
