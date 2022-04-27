#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int init_array1() {
    int a[2][2] = {
        {1, 2},
        {3, 4}};
    return a[0][0] + a[1][0];
}

int init_array2() {
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    return a[1] + a[9];
}

int init_array3() {
    char a[2][5] = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10}};
    return a[0][1] + a[1][4];
}

int init_array4() {
    int a[1][1][1][1][1][1][1][1] = {{{{{{{{1}}}}}}}};
    return a[0][0][0][0][0][0][0][0];
}

int init_array5() {
    int a[2][2] = {
        {init_array1() || 1, init_array2() - 1},
        {init_array3() + 3, init_array4() * 2}};
    return a[0][1] + a[1][1];
}

int init_array6() {
    int a[10] = {1, 0, 112};
    return a[2] + a[4] + a[5] + a[9];
}

int init_array7() {
    int a[2][2][2] = {
        {{1},
         {2}},
        {{}, {3}}};
    return a[0][0][0] + a[0][0][1] + a[1][1][0];
}

int init_array8() {
    int a[] = {1, 2, 3, 4, 5};
    return a[0] + a[4];
}

int init_array9() {
    int a[][10] = {
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {3, 4}};
    return a[7][0] + a[0][1];
}

int main() {
    ASSERT(4, init_array1(), "init_array1");
    ASSERT(12, init_array2(), "init_array2");
    ASSERT(12, init_array3(), "init_array3");
    ASSERT(1, init_array4(), "init_array4");
    ASSERT(13, init_array5(), "init_array5");
    ASSERT(112, init_array6(), "init_array6");
    ASSERT(4, init_array7(), "init_array7");
    ASSERT(6, init_array8(), "init_array8");
    ASSERT(5, init_array9(), "init_array9");

    printf("ALL TEST OF init.c SUCCESS :)\n");
    return 0;
}