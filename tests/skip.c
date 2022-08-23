#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int main() {
    /* キーワードで始まる変数名のテストも兼ねる */
    int register registerx = 10;
    ASSERT(10, registerx, "registerx");
    auto int autoy = 10;
    ASSERT(10, autoy, "autoy");
    int *const restrict volatile *restrict *volatile volatilez = 0;
    ASSERT(0, volatilez, "volatilez");
    printf("ALL TEST OF scope.c SUCCESS :)\n");
    return 0;
}