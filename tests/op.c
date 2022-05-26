#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int logical_not1() {
    int ret = 0;
    for (int i = -10; i < 10; i++) {
        if (i == 0) continue;
        if (!i) {
            ret++;
        }
    }

    return ret;
}

int logical_not2() {
    int a = 0;
    int ret = 0;
    if (!a) {
        ret += 1;
    } else {
        ret += 2;
    }

    return ret;
}

int logical_not3() {
    int a = 0;
    if (!&a) {
        return 1;
    } else {
        return 0;
    }
}

int logical_not4() {
    int a = 0;
    if (!!!!!!!!!!&a) {
        return 1;
    } else {
        return 0;
    }
}

int logical_and1() {
    return 0 && 0;
}

int logical_and2() {
    return 0 && 1;
}

int logical_and3() {
    return 1 && 0;
}

int logical_and4() {
    return 1 && 1;
}

int logical_and5() {
    int a = 2 && 100;
    int b = 0 && 131;
    return a + b;
}

int logical_or1() {
    return 0 || 0;
}

int logical_or2() {
    return 0 || 1;
}

int logical_or3() {
    return 1 || 0;
}

int logical_or4() {
    return 1 || 1;
}

int logical_or5() {
    return (0 || 100) + (100 || 0);
}

int logical_expr1() {
    return (0 && 1 == 0) || (123 > 2 && 13 || !0) && !123;
}

int termary1(int cond) {
    return cond ? 10 : 2;
}

int termary2(int cond1) {
    return termary1(cond1) - 2 ? 100 : cond1 ? 10
                                             : logical_and1();
}

int termary3() {
    int a = 1, b = 2, ans;
    ans = (a == 1 ? (b == 2 ? 3 : 5) : 0);
    return ans;
}

int and_or_xor1() {
    return 1 | 2 & 1;
}

int and_or_xor2() {
    return 1 | 2 & 1 | 1 | 2 | 12 & 1;
}

int and_or_xor3() {
    return 1 ^ 3 ^ 2;
}

int and_or_xor4() {
    return 1 & 3 ^ 10 | 2;
}

int and_or_xor5() {
    return 1 & 5 ^ !(10 | 2 + 1) * 2 / 3 ^ 2 && 1 || 7;
}

char not1() {
    char a = 1;
    return ~a;
}

int not2() {
    char a = 9;
    return ~~~~~~a + 1;
}

int main() {
    ASSERT(0, logical_not1(), "logical_not1");
    ASSERT(1, logical_not2(), "logical_not2");
    ASSERT(0, logical_not3(), "logical_not3");
    ASSERT(1, logical_not4(), "logical_not4");

    ASSERT(0, logical_and1(), "logical_and1");
    ASSERT(0, logical_and2(), "logical_and2");
    ASSERT(0, logical_and3(), "logical_and3");
    ASSERT(1, logical_and4(), "logical_and4");
    ASSERT(1, logical_and5(), "logical_and5");

    ASSERT(0, logical_or1(), "logical_or1");
    ASSERT(1, logical_or2(), "logical_or2");
    ASSERT(1, logical_or3(), "logical_or3");
    ASSERT(1, logical_or4(), "logical_or4");
    ASSERT(2, logical_or5(), "logical_or5");

    ASSERT(0, logical_expr1(), "logical_expr1");

    ASSERT(10, termary1(1), "termary1(1)");
    ASSERT(2, termary1(0), "termary1(0)");
    ASSERT(0, termary2(0), "termary2(0)");
    ASSERT(100, termary2(1), "termary2(1)");
    ASSERT(3, termary3(), "termary3");

    ASSERT(1, and_or_xor1(), "and_or_xor1");
    ASSERT(3, and_or_xor2(), "and_or_xor2");
    ASSERT(0, and_or_xor3(), "and_or_xor3");
    ASSERT(11, and_or_xor4(), "and_or_xor4");
    ASSERT(1, and_or_xor5(), "and_or_xor5");

    ASSERT(-2, not1(), "not1");
    ASSERT(10, not2(), "not2");

    printf("ALL TEST OF op.c SUCCESS :)\n");
    return 0;
}