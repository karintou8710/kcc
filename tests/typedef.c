#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

typedef int INT32;
typedef int **INT1, *INT2, INT3;
typedef int INT4, *INT5;
typedef int INT6[10], INT7[20];
typedef char CHAR;
typedef CHAR CHAR2;
typedef struct STRUCT1 {
    INT3 member;
} STRUCT1;
typedef enum ENUM1 {
    A,
    B,
    C,
    D,
} ENUM1;

INT3 typedef1() {
    INT3 a = 10;
    INT2 b = &a;
    INT1 c = &b;
    a = 30;
    return **c;
}

INT3 **typedef2() {
    INT2 a = malloc(sizeof(INT3));
    INT2 *b = &a;
    *a = 10;
    return b;
}

INT3 typedef3() {
    INT6 a;
    INT3 sum = 0;
    for (int i = 0; i < sizeof(INT6) / sizeof(int); i++) {
        a[i] = i * i;
    }
    for (int i = 0; i < sizeof(INT6) / sizeof(int); i++) {
        sum += a[i];
    }

    return sum;
}

STRUCT1 *typedef4() {
    STRUCT1 *s = malloc(sizeof(STRUCT1));
    ENUM1 e = B;
    s->member = e;
    return s;
}

int main() {
    ASSERT(4, sizeof(INT32), "sizeof(INT32)");
    ASSERT(8, sizeof(INT1), "sizeof(INT1)");
    ASSERT(8, sizeof(INT2), "sizeof(INT2)");
    ASSERT(4, sizeof(INT3), "sizeof(INT3)");
    ASSERT(4 * 10, sizeof(INT6), "sizeof(INT6)");
    ASSERT(4 * 20, sizeof(INT7), "sizeof(INT7)");
    ASSERT(1, sizeof(CHAR2), "sizeof(CHAR2)");
    ASSERT(8, sizeof((INT1)0), "sizeof((INT1)0)");

    ASSERT(30, typedef1(), "typedef1");
    ASSERT(10, **typedef2(), "typedef2");
    ASSERT(285, typedef3(), "typedef3");
    ASSERT(1, typedef4()->member, "typedef4");

    printf("ALL TEST OF enum.c SUCCESS :)\n");
    return 0;
}