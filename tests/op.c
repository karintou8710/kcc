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

// 左が0の時は右を評価しない
int logical_and6() {
    int *a = 0;
    int b = 0;
    return b && *a;
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

// 左が1の時は右を評価しない
int logical_or6() {
    int *a = 0;
    int b = 1;
    return b || *a;
}

int logical_expr1() {
    return (0 && 1 == 0) || (123 > 2 && 13 || !0) && !123;
}

int logical_expr2() {
    int *a = 0;
    return ((1 || *a) == 0 && (*a || 1 && *a)) || 1;
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

int shift1() {
    int a = 1 << 2;
    return a << a;
}

int shift2() {
    int a = 1 << 2 >> 2;
    return a;
}

int equal1() {
    int a = 1;
    a &= 3;
    return a;
}

int equal2() {
    int a = 1;
    a |= 3;
    return a;
}

int equal3() {
    int a = 1;
    a ^= 3;
    return a;
}

int equal4() {
    int a = 1;
    a <<= 3;
    return a;
}

int equal5() {
    int a = 10;
    a >>= 2;
    return a;
}

int cumma1() {
    int a, b, c;
    a = 1, b = 2, c = 3;
    return a + b + c;
}

int cumma2() {
    int a, b, c = 3;
    int d = (a = 1, b = 2, c) * 2;
    return d;
}

int cumma3() {
    struct A {
        int a;
    } a, b, c;
    a.a = 10, b.a = 20, c.a = 30;
    return (a, b, c).a;
}

// extern test
typedef struct FILE FILE;
extern FILE *stderr, stdout;

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
    ASSERT(0, logical_and6(), "logical_and6");

    ASSERT(0, logical_or1(), "logical_or1");
    ASSERT(1, logical_or2(), "logical_or2");
    ASSERT(1, logical_or3(), "logical_or3");
    ASSERT(1, logical_or4(), "logical_or4");
    ASSERT(2, logical_or5(), "logical_or5");
    ASSERT(1, logical_or6(), "logical_or6");

    ASSERT(0, logical_expr1(), "logical_expr1");
    ASSERT(1, logical_expr2(), "logical_expr2");

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

    ASSERT(64, shift1(), "shift1");
    ASSERT(1, shift2(), "shift2");

    ASSERT(1, equal1(), "equal1");
    ASSERT(3, equal2(), "equal2");
    ASSERT(2, equal3(), "equal3");
    ASSERT(8, equal4(), "equal4");
    ASSERT(2, equal5(), "equal5");

    ASSERT(6, cumma1(), "cumma1");
    ASSERT(6, cumma2(), "cumma2");
    ASSERT(30, cumma3(), "cumma3");
    ASSERT(6, (1, 3, 6), "(1,3,6)");

    ASSERT(0, ({ 0; }), "({0;})");
    ASSERT(10, ({int a;a = 10;a; }), "({int a;a = 10;a; })");
    ASSERT(5, ({ 3; }) * ({ 2; }) - ({ 1; }), "({3;}) * ({2;}) - ({1})");

    ASSERT(1, stderr != 0, "stderr != NULL");
    ASSERT(1, stdout != 0, "stdout != NULL");

    printf("ALL TEST OF op.c SUCCESS :)\n");
    return 0;
}