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

// 配列を戻り値とするのは未対応
int main() {
    ASSERT(15, return_type_cast1(), "return_type_cast1");
    ASSERT(0, return_type_cast2(), "return_type_cast2");
    ASSERT(1, return_type_cast3(), "return_type_cast3");
    ASSERT(1, assign_type_cast1(), "assign_type_cast1");
    ASSERT(127, assign_type_cast2(), "assign_type_cast2");
    ASSERT(1, assign_type_cast3(), "assign_type_cast3");
    ASSERT(1, assign_type_cast4(), "assign_type_cast4");

    printf("ALL TEST OF type.c SUCCESS :)\n");
    return 0;
}