#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int goto1() {
    int res = 0;
test1:
    if (res == 0) {
        res = 1;
        goto test1;
        res = 2;
    }

    return res;
}

int goto2() {
    int res = 0;
    goto test2;
    res = 1;
test2:
    return res;
}

int goto3() {
    int res = 0;
    int i = 4;
    goto test3;
    for (; i < 10; i++) {
        res++;
    test3:;
    }
    return res;
}

int main() {
    ASSERT(1, goto1(), "goto1");
    ASSERT(0, goto2(), "goto2");
    ASSERT(5, goto3(), "goto3");

    printf("ALL TEST OF goto.c SUCCESS :)\n");
    return 0;
}