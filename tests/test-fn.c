#include <stdio.h>
#include <stdlib.h>

int ret() {
    return 3;
}

void p() {
    printf("Hello, World\n");
}

void int_print(int d) {
    printf("%d\n", d);
}

int constant(int a) {
    return a;
}

int add(int a, int b) {
    return a + b;
}

int add6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

void alloc4(int **p, int a, int b, int c, int d) {
    *p = malloc(sizeof(int) * 5);
    (*p)[0] = a;
    (*p)[1] = b;
    (*p)[2] = c;
    (*p)[3] = d;
    (*p)[4] = 0;
}