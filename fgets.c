/*
    module  : fgets.c
    version : 1.1
    date    : 09/12/20
*/
#define EXTERN
#include "my_files.h"

/*
    my_fill fills my_buf from fd.
    There is no seek_pos.
*/
static void my_fill(long fd)
{
    max_pos[fd] = my_read(fd, my_buf[fd], MAXBUF);
    cur_pos[fd] = 0;
}

/*
    my_fgetc behaves like fgetc.
    There is no unget.
*/
long my_fgetc(long fd)
{
    if (cur_pos[fd] == max_pos[fd])
	my_fill(fd);
    if (!max_pos[fd])
	return -1;
    return my_buf[fd][cur_pos[fd]++];
}

/*
    my_fgets reads from fd; returns 0 on success or -1 on end-of-file.
*/
long my_fgets(char *str, long size, long fd)
{
    long ch;

    if ((ch = my_fgetc(fd)) == -1)
	return -1;
    while (--size > 0) {
	if ((*str++ = ch) == '\n')
	    break;
	if ((ch = my_fgetc(fd)) == -1)
	    break;
    }
    *str = 0;
    return 0;
}
