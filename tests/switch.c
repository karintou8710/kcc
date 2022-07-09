#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int switch1(int x) {
    int res = 0;
    switch (x) {
        case 1:
            res++;
        case 2:
            res++;
        case 3:
            res++;
            break;
        case 4:
            res++;
        default:
            res++;
    }
    return res;
}

int switch2(int x) {
    int res = 0;
    switch (x + 1) {
        case 1: {
            res++;
            break;
        }
        case 1 * 2: {
            switch (x) {
                case 1: {
                    break;
                }
                    res++;
            }
            res++;
        }
            res++;
    }
    return res;
}

int switch3(int x) {
    int res = 0;
    switch (x) {
        default:
            for (int i = 0; i < 10; i++) {
                res++;
                if (i == 5) break;
            }
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (res > 10) break;
                    res++;
                }
                case 10:
                    res++;
            }
            break;
            res = -1;
    }
    return res;
}

int switch4() {
    char *p1 = calloc(20, sizeof(char));
    char *p2 = "Hello,world";
    char *to = p1;
    char *from = p2;
    int count = 12;

    switch (count % 8) {
        case 0:
            do {
                *to++ = *from++;
                case 7:
                    *to++ = *from++;
                case 6:
                    *to++ = *from++;
                case 5:
                    *to++ = *from++;
                case 4:
                    *to++ = *from++;
                case 3:
                    *to++ = *from++;
                case 2:
                    *to++ = *from++;
                case 1:
                    *to++ = *from++;
            } while ((count -= 8) > 0);
    }
    return strcmp(p1, p2);
}

int main() {
    ASSERT(1, switch1(-1), "switch1(-1)");
    ASSERT(1, switch1(0), "switch1(0)");
    ASSERT(3, switch1(1), "switch1(1)");
    ASSERT(2, switch1(2), "switch1(2)");
    ASSERT(1, switch1(3), "switch1(3)");
    ASSERT(2, switch1(4), "switch1(4)");
    ASSERT(1, switch1(5), "switch1(5)");
    ASSERT(1, switch1(6), "switch1(6)");
    ASSERT(1, switch2(0), "switch2(0)");
    ASSERT(2, switch2(1), "switch2(1)");
    ASSERT(0, switch2(2), "switch2(2)");
    ASSERT(21, switch3(0), "switch3(0)");
    ASSERT(1, switch3(10), "switch3(10)");
    ASSERT(0, switch4(), "switch4()");

    printf("ALL TEST OF switch.c SUCCESS :)\n");
    return 0;
}