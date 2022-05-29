#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int test1();
int test1();
int test1() {
    return 0;
}

int test2(int a);
int test2(int);
int test2(int a) {
    return a;
}
int test2(int);

int test3(int *, int[2][2]);
int test3(int *a, int b[2][2]) {
    return *a + b[0][1];
}

void test4(void);

int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
void swap(void **p, void **q);

int main() {
    ASSERT(0, test1(), "test1");
    ASSERT(1, test2(1), "test2");

    int a = 1;
    int b[2][2] = {{1, 2}, {3, 4}};
    ASSERT(3, test3(&a, &b), "test3");

    printf("ALL TEST OF prototype.c SUCCESS :)\n");
    return 0;
}
