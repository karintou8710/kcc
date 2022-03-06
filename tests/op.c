#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name)
{
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int logical_not1() {
    int ret = 0;
    for (int i=-10;i<10;i++) {
        if (i==0) continue;
        if (!i) {
            ret++;
        }
    }

    return ret;
}

int logical_not2() {
    int a = 0;
    int ret = 0;
    if (!a) {
        ret += 1;
    } else {
        ret += 2;
    }

    return ret;
}

int logical_not3() {
    int a = 0;
    if (!&a) {
        return 1;
    } else {
        return 0;
    }
}

int logical_not4() {
    int a = 0;
    if (!!!!!!!!!!&a) {
        return 1;
    } else {
        return 0;
    }
}

int main() {

    ASSERT(0, logical_not1(), "logical_not1");
    ASSERT(1, logical_not2(), "logical_not2");
    ASSERT(0, logical_not3(), "logical_not3");
    ASSERT(1, logical_not4(), "logical_not4");

    printf("ALL TEST OF op.c SUCCESS :)\n");
    return 0;
}