#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int main() {
    ASSERT(7, '\a', "\\a");
    ASSERT(92, '\\', "\\");
    ASSERT(39, '\'', "'");

    char buf[100] = {'\\', '"', '\\', '"', 0};
    char buf2[100] = "\"\"";
    ASSERT(strcmp(buf2, buf), 0, "\"\"");

    printf("ALL TEST OF escape.c SUCCESS :)\n");
    return 0;
}