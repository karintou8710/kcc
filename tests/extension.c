#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int new1() {
    int *a = new (int), b = 10;
    a = &b;
    *a = 20;
    return b;
}

int new2() {
    int *a = new (int[10]) + 1;
    int *b = a - 1;
    *(a - 1) = 10;
    *(a) = 20;
    return b[0] == 10 && b[1] == 20;
}

int main() {
    ASSERT(20, new1(), "new1()");
    ASSERT(1, new2(), "new2()");

    printf("ALL TEST OF escape.c SUCCESS :)\n");
    return 0;
}