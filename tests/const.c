#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

unsigned typedef long long const int A;

int const1() {
    const int a = 1;
    A b = 2;
    return a + b;
}

int const2() {
    int b = 1;
    int *const a = &b;
    *a = 10;
    return *a;
}

int const3() {
    const struct A { int member; } a;
    struct A b;
    b = a;
    b.member = 10;
    return b.member;
}

int main() {
    ASSERT(3, const1(), "const1()");
    ASSERT(10, const2(), "const2()");
    ASSERT(10, const3(), "const3()");

    printf("ALL TEST OF const.c SUCCESS :)\n");
    return 0;
}