#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

// basic
union A1 {
    char a;
    int b;
};

// offset
int union1() {
    union A1 test;
    return &test.a == &test.b;
}

// array, alignment
union A2 {
    char a[2];
    char b[13];
    long c;
} a2_test;

// array, access
int union2() {
    a2_test.a[0] = 1;
    a2_test.a[1] = 2;
    return a2_test.b[0] == 1 && a2_test.b[1] == 2;
}

// pointer
int union3() {
    union A2 *test, test2;
    test2.a[0] = 1;
    test = &test2;
    return test->b[0];
}

// assign
int union4() {
    union A2 t1, t2;
    t1.a[0] = 10;
    t1.a[1] = 20;
    t2 = t1;
    return t2.b[0] == 10 && t2.b[1] == 20;
}

// typedef
typedef union A2 A2;
int union5() {
    A2 test;
    test.b[0] = 1;
    return test.a[0];
}

// union in struct
struct S1 {
    int a;
    union A2 a2;
    int c;
};
int union6() {
    struct S1 s;
    A2 a2;
    s.a = 10;
    a2.a[0] = 1;
    s.a2 = a2;
    s.c = 11;
    return s.a == 10 && s.a2.b[0] == 1 && s.c == 11;
}

// prototype
int union7() {
    union A3 *a;
    union A3 {
        int a;
        char b;
    } b;
    a = &b;
    a->a = 1;
    return b.a;
}

// global and local scope
int union8() {
    union A2 {
        int member;
    };
    union A2 test;
    test.member = 1;
    return test.member;
}

int main() {
    ASSERT(4, sizeof(union A1), "sizeof(union A1)");
    ASSERT(16, sizeof(union A2), "sizeof(union A2)");
    ASSERT(32, sizeof(struct S1), "sizeof(struct S1)");

    ASSERT(1, union1(), "union1()");
    ASSERT(1, union2(), "union2()");
    ASSERT(1, union3(), "union3()");
    ASSERT(1, union4(), "union4()");
    ASSERT(1, union5(), "union5()");
    ASSERT(1, union6(), "union6()");
    ASSERT(1, union7(), "union7()");
    ASSERT(1, union8(), "union8()");

    printf("ALL TEST OF union.c SUCCESS :)\n");
    return 0;
}