#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int enum1(int index) {
    enum A {
        A,
        B,
        C,
        D,
        E
    };
    if (index == 0)
        return A;
    else if (index == 1)
        return B;
    else if (index == 2)
        return C;
    else if (index == 3)
        return D;
    else if (index == 4)
        return E;
}

int enum2(int index) {
    enum A {
        A = 10,
        B,
        C,
    };
    if (index == 0)
        return A;
    else if (index == 1)
        return B;
    else if (index == 2)
        return C;
}

int enum3(int index) {
    enum A {
        A = 10,
        B = 30,
        C = -29,
        D,
        E = 51 * 2
    };
    if (index == 0)
        return A;
    else if (index == 1)
        return B;
    else if (index == 2)
        return C;
    else if (index == 3)
        return D;
    else if (index == 4)
        return E;
}

int enum4(int index) {
    enum {
        A = 10,
    };
    enum {
        B = 20,
    };
    enum {
        C = 30,
    };
    if (index == 0)
        return A;
    else if (index == 1)
        return B;
    else if (index == 2)
        return C;
}

int enum5() {
    enum TEST { A,
                B,
                C,
                D };
    return sizeof(A) + sizeof(enum TEST);
}

int enum6() {
    enum A { A,
             B,
             C };
    enum A y;
    return sizeof(y);
}

int enum7() {
    enum color { RED,
                 GREEN,
                 BLUE } c = RED,
                        *cp = &c;
    return *cp;
}

int enum8() {
    enum TEST {
        a = 1 << 1,
        b = 1 << 2
    };
    return a;
}

int main() {
    ASSERT(0, enum1(0), "enum1(0)");
    ASSERT(1, enum1(1), "enum1(1)");
    ASSERT(2, enum1(2), "enum1(2)");
    ASSERT(3, enum1(3), "enum1(3)");
    ASSERT(4, enum1(4), "enum1(4)");

    ASSERT(10, enum2(0), "enum2(0)");
    ASSERT(11, enum2(1), "enum2(1)");
    ASSERT(12, enum2(2), "enum2(2)");

    ASSERT(10, enum3(0), "enum3(0)");
    ASSERT(30, enum3(1), "enum3(1)");
    ASSERT(-29, enum3(2), "enum3(2)");
    ASSERT(-28, enum3(3), "enum3(3)");
    ASSERT(102, enum3(4), "enum3(4)");

    ASSERT(10, enum4(0), "enum4(0)");
    ASSERT(20, enum4(1), "enum4(1)");
    ASSERT(30, enum4(2), "enum4(2)");

    ASSERT(8, enum5(), "enum5");
    ASSERT(4, enum6(), "enum6");
    ASSERT(0, enum7(), "enum7");
    ASSERT(2, enum8(), "enum8");

    printf("ALL TEST OF enum.c SUCCESS :)\n");
    return 0;
}