#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int for1() {
    int res = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            res++;
        }
        for (int k = 0; k < 2; k++) {
            for (int ii = 0; ii < 5; ii++) {
                res++;
            }
            for (int jj = 0; jj < 5; jj++) {
                res++;
            }
        }
    }

    return res;
}

// pushとpopの数の違いによるバグのテスト
int while1() {
    int a = 1;
    while (a == '*') {
    }
    return a;
}

int break_for1() {
    int res = 0;
    for (int i = 0; i < 100; i++) {
        if (i == 10) break;
        res++;
    }
    return res;
}

int break_for2() {
    int res = 0;
    for (int i = 0; i < 10; i++) {
        if (i == 2) {
            res += i;
            break;
        }
        for (int j = 0; j < 10; j++) {
            if (j == 1) {
                res += j;
                break;
            }
            for (int k = 0; k < 10; k++) {
                if (k == 5) {
                    res += k;
                    break;
                }
            }
        }
    }
    return res;
}

int break_while1() {
    int i = 0;
    while (1) {
        if (i == 10) break;
        i++;
    }
    return i;
}

int break_while2() {
    int i = 0;
    int j = 0;
    int k = 0;
    while (1) {
        if (i == 5) break;
        while (1) {
            if (j == 5) break;
            while (1) {
                if (k == 5) break;
                k++;
            }
            j++;
        }
        i++;
    }
    return i + j + k;
}

int break_for_loop1() {
    int j = 0;
    int i = 0;
    for (i = 0; i < 10; i++) {
        if (i == 5) break;
        while (1) {
            if (j == 3) break;
            j++;
        }
    }
    return i + j;
}

int continue_for1() {
    int res = 0;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        res += i;
    }
    // 1 + 3 + 5 + 7 + 9 = 25
    return res;
}

int continue_for2() {
    int res = 0;
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) continue;
        res++;
        for (int j = 0; j < 3; j++) {
            if (j % 2 == 0) continue;
            res++;
            for (int k = 0; k < 3; k++) {
                if (k % 2 == 0) continue;
                res++;
            }
        }
    }

    return res;
}

int continue_for3() {
    int res = 0;
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) continue;
        res++;
        for (int j = 0; j < 3; j++) {
            if (j % 2 == 0) continue;
            res++;
            for (int k = 0; k < 3; k++) {
                if (k % 2 == 0) continue;
                res++;
            }
        }

        for (int ii = 0; ii < 5; ii++) {
            if (ii % 2 == 0) continue;
            res++;
        }
    }

    return res;
}

int continue_for4() {
    int res = 0;
    for (int i = 0; i < 5; i++) {
        res++;
        for (int j = 0; j < 5; j++) {
            res++;
        }

        for (int j = 0; j < 5; j++) {
            res++;
            continue;
        }

        res++;
        continue;
    }
    return res;
}

int continue_while1() {
    int i = 0;
    int res = 0;
    while (1) {
        i++;
        if (i == 10) break;
        if (i % 2 == 0) continue;
        res += i;
    }
    return res;
}

int continue_while2() {
    int i = 0;
    int j = 0;
    int k = 0;
    int res = 0;

    while (i < 5) {
        if (i % 2 == 0) {
            i++;
            continue;
        }
        while (j < 5) {
            if (j % 2 == 0) {
                j++;
                continue;
            }
            j++;
            res++;
        }
        while (k < 5) {
            if (k % 2 == 0) {
                k++;
                continue;
            }
            k++;
            res++;
        }
        i++;
    }

    return res;
}

int complex_loop1() {
    int res = 0;
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) continue;
        res++;
        for (int j = 0; j < 3; j++) {
            if (j % 2 == 0) continue;
            res++;
            for (int k = 0; k < 3; k++) {
                if (k % 2 == 0) continue;
                res++;
            }
        }

        for (int ii = 0; ii < 5; ii++) {
            if (ii % 2 == 0) continue;
            res++;
        }

        int i1 = 0;
        int j1 = 0;
        int k1 = 0;

        while (i1 < 5) {
            if (i1 % 2 == 0) {
                i1++;
                continue;
            }
            while (j1 < 5) {
                if (j1 % 2 == 0) {
                    j1++;
                    continue;
                }
                j1++;
                res++;
            }
            while (k1 < 5) {
                if (k1 % 2 == 0) {
                    k1++;
                    continue;
                }
                k1++;
                res++;
            }
            i1++;
        }
    }

    return res;
}

int do_while1() {
    int a = 0;
    do {
        a = 10;
    } while (0);
    return a;
}

int do_while2() {
    int a = 10;
    do {
        if (a == 5) break;
        a--;
    } while (a > 0);
    return a;
}

int do_while3() {
    int a = 10, res = 0;
    do {
        a--;
        if (a % 2 == 0) continue;
        res++;
    } while (a > 0);
    return res;
}

int do_while4() {
    int i = 0;
    int res = 0;
    do {
        int j = 0;
        do {
            int sum = 0;
            while (sum < 10) {
                sum += 1;
                if (sum % 5 == 0) break;
            }
            res += sum;
            j++;
            if (res > 30) break;
        } while (j < 5);
        i++;
        for (int k = 0; k < 10; k++) {
            res++;
        }
        for (int k = 0; k < 10; k++) {
            res++;
            if (res > 70) break;
        }
        if (res > 100) break;
    } while (i < 5);
    return res;
}

int main() {
    ASSERT(66, for1(), "for1");
    ASSERT(1, while1(), "while1");

    ASSERT(10, break_for1(), "break_for1");
    ASSERT(14, break_for2(), "break_for2");
    ASSERT(10, break_while1(), "break_while1");
    ASSERT(15, break_while2(), "break_while2");
    ASSERT(8, break_for_loop1(), "break_for_loop1");

    ASSERT(25, continue_for1(), "continue_for1");
    ASSERT(3, continue_for2(), "continue_for2");
    ASSERT(5, continue_for3(), "continue_for3");
    ASSERT(60, continue_for4(), "continue_for4");
    ASSERT(25, continue_while1(), "continue_while1");
    ASSERT(4, continue_while2(), "continue_while2");

    ASSERT(9, complex_loop1(), "complex_loop1");

    ASSERT(10, do_while1(), "do_while1");
    ASSERT(5, do_while2(), "do_while2");
    ASSERT(5, do_while3(), "do_while3");
    ASSERT(102, do_while4(), "do_while4");

    printf("ALL TEST OF test.c SUCCESS :)\n");
    return 0;
}