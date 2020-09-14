/*
    module  : my_files.h
    version : 1.1
    date    : 09/12/20
*/
#ifndef _FILES_H
#define _FILES_H

#include "my_libc.h"

#define MAXFIL		10
#define MAXSTR		80
#define MAXBUF		512

EXTERN char my_buf[MAXFIL][MAXBUF];
EXTERN long cur_pos[MAXFIL], max_pos[MAXFIL];
#endif
