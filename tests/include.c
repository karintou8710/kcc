#include <stdio.h>

#include "tests/empty.h"
#include "tests/test.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int init() {
    test = 1;
    test2 = &test;
    a.member = 10;
    b.member = 20;
}

int main() {
    init();

    ASSERT(1, *test2, "*test2");
    ASSERT(10, a.member, "a.member");
    ASSERT(20, b.member, "b.member");
    ASSERT(20, NUM2, "NUM2");

    printf("ALL TEST OF include.c SUCCESS :)\n");

    return 0;
}