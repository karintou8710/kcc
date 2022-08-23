#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

// string_literal
int string_literal1() {
    char *a;
    a = "Hello, C-compiler";
    printf("%s\n", a);
    return 0;
}

int string_literal2() {
    char a[] =
        "Hel"
        "lo"
        ","
        "w"
        "orl"
        ""
        "d";
    return sizeof(a) == 12 && strcmp(a, "Hello,world") == 0;
}

int string_literal3() {
    char a[] = "";
    return sizeof(a) == 1 && a[0] == '\0';
}

int main() {
    ASSERT(0, string_literal1(), "string_literal1");
    ASSERT(1, string_literal2(), "string_literal2");
    ASSERT(1, string_literal3(), "string_literal3");

    printf("ALL TEST OF string.c SUCCESS :)\n");
    return 0;
}