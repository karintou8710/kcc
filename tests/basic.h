#ifndef BASIC_H
#define BASIC_H

typedef long size_t;
typedef struct __builtin_va_list {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

void va_start(va_list ap, char *fmt);
int printf(char *format, ...);
void exit(int status);
int strcmp(char *s1, char *s2);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
int puts(char *s);
int vsprintf(char *s, char *format, va_list arg);

#endif