# 141 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4

#include <stdio.h>
#include <string.h>

# 1 "/usr/include/x86_64-linux-gnu/bits/typesizes.h" 1 3 4
# 142 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/time64.h" 1 3 4

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int main() {
    printf("ALL TEST OF preprocess.c SUCCESS :)\n");
    return 0;
}