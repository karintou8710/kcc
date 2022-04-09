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

int logical_and1() {
    return 0 && 0;
}

int logical_and2() {
    return 0 && 1;
}

int logical_and3() {
    return 1 && 0;
}

int logical_and4() {
    return 1 && 1;
}

int logical_and5() {
    int a = 2 && 100;
    int b = 0 && 131;
    return a+b;
}

int logical_or1() {
    return 0 || 0;
}

int logical_or2() {
    return 0 || 1;
}

int logical_or3() {
    return 1 || 0;
}

int logical_or4() {
    return 1 || 1;
}

int logical_or5() {
    return (0 || 100) + (100 || 0);
}

int logical_expr1() {
    return (0 && 1==0) || (123> 2 && 13 || !0) && !123;
}

int main() {

    ASSERT(0, logical_not1(), "logical_not1");
    ASSERT(1, logical_not2(), "logical_not2");
    ASSERT(0, logical_not3(), "logical_not3");
    ASSERT(1, logical_not4(), "logical_not4");

    ASSERT(0, logical_and1(), "logical_and1");
    ASSERT(0, logical_and2(), "logical_and2");
    ASSERT(0, logical_and3(), "logical_and3");
    ASSERT(1, logical_and4(), "logical_and4");
    ASSERT(1, logical_and5(), "logical_and5");

    ASSERT(0, logical_or1(), "logical_or1");
    ASSERT(1, logical_or2(), "logical_or2");
    ASSERT(1, logical_or3(), "logical_or3");
    ASSERT(1, logical_or4(), "logical_or4");
    ASSERT(2, logical_or5(), "logical_or5");

    ASSERT(0, logical_expr1(), "logical_expr1");

    printf("ALL TEST OF op.c SUCCESS :)\n");
    return 0;
}