#include <stdio.h>
#include <string.h>

extern char __va_area__[136];

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

// 組み込み変数

typedef struct __builtin_va_list {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

char *variadic1(char *fmt, ...) {
    char buf[100];
    va_list ap;
    *ap = *(struct __builtin_va_list *)__va_area__;
    vsprintf(buf, fmt, ap);
    return buf;
}

char *variadic2(char *fmt, ...) {
    char buf[100];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    return buf;
}

int main() {
    ASSERT(0, strcmp(variadic1("%d %d %s", 10, 20, "hello"), "10 20 hello"), "variadic1()");
    ASSERT(0, strcmp(variadic1("%d/%d/%s", 10000, -200, ""), "10000/-200/"), "variadic1()");
    ASSERT(0, strcmp(variadic2("%d %d %s", 10, 20, "hello"), "10 20 hello"), "variadic2()");
    ASSERT(0, strcmp(variadic2("%d/%d/%s", 10000, -200, ""), "10000/-200/"), "variadic2()");

    printf("ALL TEST OF variadic.c SUCCESS :)\n");
    return 0;
}
