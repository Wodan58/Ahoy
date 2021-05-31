/*
    module  : my_struc.h
    version : 1.1
    date    : 05/31/21
*/
#ifndef MY_STRUC_H
#define MY_STRUC_H

typedef struct idx_t {
    int reg;
    long scale;
} idx_t;

typedef struct name_t {
    char *str;
    long offset;
} name_t;

typedef struct reg_t {
    int reg;
    long num;
    idx_t idx;
} reg_t;

typedef struct mem_t {
    int tag;
    long size;
    union {
        name_t name;
        reg_t reg;
    };
} mem_t;

typedef struct jmp_t {
    int tag;
    union {
        int reg;
        char *str;
        mem_t mem;
    };
} jmp_t;
#endif
