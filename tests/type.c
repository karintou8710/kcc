#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int *_return_type_cast1_sub() {
    int *a = malloc(4);
    *a = 10;
    return a;
}

int return_type_cast1() {
    int *a = _return_type_cast1_sub();
    *a += 5;
    return *a;
}

void _return_type_cast2_sub() {
    int a = 1;
    int b = 2;
    return;
}

// 実行できるかテスト
int return_type_cast2() {
    _return_type_cast2_sub();
    return 0;
}

char _return_type_cast3_sub() {
    return 257;  // 1,00000001
}

int return_type_cast3() {
    int b = _return_type_cast3_sub();
    return b;
}

char assign_type_cast1() {
    char a = 257;
    return a;
}

int assign_type_cast2() {
    char a = 127;
    int b = a;
    return b;
}

int assign_type_cast3() {
    int a = 257;
    char b = a;
    return b;
}

int *assign_type_cast4() {
    char a = 1;
    int *b = a;
    return b;
}

int pointer_sub1() {
    int a = 1;
    int *p1 = &a, *p2 = &a;
    p2 += 3;
    return p2 - p1;
}

int pointer_sub2() {
    int a[100];
    return a + (3) - a;
}

int pointer_sub3() {
    int a[100];
    int *b = a + 10;
    return b + 1 - a;
}

long long1() {
    long a = 10;
    a += 1000000000000000;
    return a;
}

long long2() {
    long a = 100000000000127;
    return (long)(char)a;
}

long long long3() {
    long long int a = 100000000000000;
    long long b = 100000000000000;
    long int c = 100000000000000;
    return a + b + c;
}

short short1() {
    short a = (1 << 14) + (1 << 20);
    return a;
}

short short2() {
    short a = (1 << 14) + 127;
    return (short)(char)a;
}

int cast5() {
    int a[10];
    int *b = (int *)a;
    *b = 10;
    return a[0];
}

int cast6() {
    int a[2];
    struct A {
        int m1;
        int m2;
    };
    a[0] = 100;
    a[1] = 200;
    struct A c = *(struct A *)a;
    return (c.m1 == 100) && (c.m2 == 200);
}

// 配列を戻り値とするのは未対応
int main() {
    ASSERT(15, return_type_cast1(), "return_type_cast1");
    ASSERT(0, return_type_cast2(), "return_type_cast2");
    ASSERT(1, return_type_cast3(), "return_type_cast3");
    ASSERT(1, assign_type_cast1(), "assign_type_cast1");
    ASSERT(127, assign_type_cast2(), "assign_type_cast2");
    ASSERT(1, assign_type_cast3(), "assign_type_cast3");
    ASSERT(1, assign_type_cast4(), "assign_type_cast4");
    ASSERT(3, pointer_sub1(), "pointer_sub1");
    ASSERT(3, pointer_sub2(), "pointer_sub2");
    ASSERT(11, pointer_sub3(), "pointer_sub3");

    ASSERT(0, (char)(256), "cast1");
    ASSERT(4, (char)((1 << 30) + (1 << 2)), "cast2");
    ASSERT(0, (int)(char)(char *)(256), "cast3");
    ASSERT(100, (char)(256 + 10 * 10), "cast4");
    ASSERT(10, cast5(), "cast5");
    ASSERT(1, cast6(), "cast6");

    ASSERT(1000000000000010, long1(), "long1");
    ASSERT(127, long2(), "long2");
    ASSERT(300000000000000, long3(), "long3");

    ASSERT(1 << 14, short1(), "short1");
    ASSERT(127, short2(), "short2");

    printf("ALL TEST OF type.c SUCCESS :)\n");
    return 0;
}