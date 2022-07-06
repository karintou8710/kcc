#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}


int init_array8() {
    int a[] = {1, 2, 3, 4, 5};
    return a[0] + a[4];
}

int test9[] = {1, 2, 3};


int main() {
    ASSERT(6, init_array8(), "init_array8");

    ASSERT(2, test9[1], "test9[1]");

    printf("ALL TEST OF init2.c SUCCESS :)\n");
    return 0;
}