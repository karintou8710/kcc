#ifndef STDARG_H
#define STDARG_H

typedef struct __builtin_va_list {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

void va_start(va_list ap, char *fmt);

#endif