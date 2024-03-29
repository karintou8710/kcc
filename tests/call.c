#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

long _test_func1() {
    return 0;
}

struct A {
    long (*f)();
};

int call1() {
    return ((((_test_func1))))();
}

int call2() {
    struct A a;
    a.f = _test_func1;
    return (a).f();
}

long call3() {
    struct A a;
    a.f = _test_func1;
    return sizeof(a.f());
}

int main() {
    ASSERT(0, call1(), "call1");
    ASSERT(0, call2(), "call2");
    ASSERT(8, call3(), "call3");

    printf("ALL TEST OF call.c SUCCESS :)\n");
    return 0;
}