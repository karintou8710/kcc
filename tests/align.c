#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

struct A1 {
    int a;
    char b;
};

struct A2 {
    char a;
    long b;
};

struct A3 {
    int a;
    int *b;
    char c;
    long d;
    int e;
};

struct A4 {
    char a;
    struct A3 b;
};

struct A5 {
    char a;
    struct A3 *b;
    struct A5 *c;
};

struct A6 {
    int a[2];
    char b[2][2][1];
    long c;
    char *d[1];
    char e;
};

int main() {
    ASSERT(8, sizeof(struct A1), "sizeof(struct A1)");
    ASSERT(16, sizeof(struct A2), "sizeof(struct A2)");
    ASSERT(40, sizeof(struct A3), "sizeof(struct A3)");
    ASSERT(48, sizeof(struct A4), "sizeof(struct A4)");
    ASSERT(24, sizeof(struct A5), "sizeof(struct A5)");
    ASSERT(40, sizeof(struct A6), "sizeof(struct A6)");

    ASSERT(1, _Alignof(char), "_Alignof(char)");
    ASSERT(2, _Alignof(short), "_Alignof(short)");
    ASSERT(4, _Alignof(int), "_Alignof(int)");
    ASSERT(8, _Alignof(long), "_Alignof(long)");
    ASSERT(8, _Alignof(long long int), "_Alignof(long)");
    ASSERT(1, _Alignof(char[2][2]), "_Alignof(char[2][2])");
    ASSERT(8, _Alignof(char(*)[2][2]), "_Alignof(char(*)[2][2])");
    ASSERT(4, _Alignof(struct A1), "_Alignof(struct A1)");
    ASSERT(8, _Alignof(struct A2), "_Alignof(struct A2)");
    ASSERT(8, _Alignof(struct A3), "_Alignof(struct A3)");
    ASSERT(8, _Alignof(struct A4), "_Alignof(struct A4)");
    ASSERT(8, _Alignof(struct A5), "_Alignof(struct A5)");
    ASSERT(8, _Alignof(struct A6), "_Alignof(struct A6)");

    printf("ALL TEST OF align.c SUCCESS :)\n");
    return 0;
}