#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int main() {
    ASSERT(0, 0b0, "0b0");
    ASSERT(1, 0b1, "0b1");
    ASSERT(1028, 0b10000000100, "0b10000000100");
    ASSERT(4, 0b000000100, "0b000000100");
    ASSERT(3, 0b1 * 0b11, "0b1 * 0b11");

    ASSERT(10, 0xa, "0xa");
    ASSERT(0, 0x0, "0x0");
    ASSERT(43788, 0x0ab0c, "0x0ab0c");
    ASSERT(1, 0b00010100 == 0x14, "0b00010100 == 0x14");

    ASSERT(0, 00, "00");
    ASSERT(8, 010, "010");
    ASSERT(72, 000110, "000110");
    ASSERT(1, 0b100100 == 044, " 0b100100 == 044");

    printf("ALL TEST OF base_num.c SUCCESS :)\n");
    return 0;
}