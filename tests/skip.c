#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int main() {
    int register x = 10;
    ASSERT(10, x, "x");
    auto int y = 10;
    ASSERT(10, y, "y");
    int *const restrict volatile *restrict *volatile z = 0;
    ASSERT(0, z, "z");
    printf("ALL TEST OF scope.c SUCCESS :)\n");
    return 0;
}