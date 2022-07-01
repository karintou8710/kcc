#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int struct1() {
    struct A {
        int num;
        int num2;
        int num3;
        char num4;
    };
    struct A p;
    p.num = 1;
    p.num2 = 2;
    p.num3 = 3;
    p.num4 = 4;
    return p.num - p.num2 + p.num3 * p.num4;
}

int struct2() {
    struct A {
        int num;
        int num2;
        int num3;
    };
    struct A *p;
    p = malloc(12);
    (*p).num = 1;
    (*p).num2 = 29;
    (*p).num3 = 3;
    return (*p).num * (*p).num2 % (*p).num3;
}

int struct3() {
    struct A {
        int num;
    };
    struct A **p2, *p1, p0;
    p0.num = 1;
    p1 = &p0;
    p2 = &p1;
    return (**p2).num;
}

// 構造体の配列
int struct4() {
    struct A {
        int num;
    };
    struct A p[10];
    int i;
    for (i = 0; i < 10; i++) {
        p[i].num = i;
    }

    int sum = 0;
    for (i = 0; i < 10; i++) {
        sum += p[i].num;
    }
    return sum;
}

// 配列をポインターに代入
int struct5() {
    struct A {
        int num;
        int num2;
    };
    struct A p[10], *p2;
    p2 = p;
    int i;
    for (i = 0; i < 10; i++) {
        p2[i].num2 = i;
    }

    int sum = 0;
    for (i = 0; i < 10; i++) {
        sum += p2[i].num2;
    }
    return sum;
}

// 多次元配列
int struct6() {
    struct A {
        int num;
        int num2;
    };
    struct A p[5][5][5];
    int i, j, k;
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            for (k = 0; k < 5; k++) {
                p[i][j][k].num = i;
                p[i][j][k].num2 = j + k;
            }
        }
    }

    return p[1][2][3].num + p[2][3][4].num2;
}

// メンバーに配列を持つ
int struct7() {
    struct A {
        int num;
        int num2[10];
        int num3;
    };

    struct A p;

    int i;
    for (i = 0; i < 10; i++) {
        p.num2[i] = i;
    }

    int sum = 0;
    for (i = 0; i < 10; i += 2) {
        sum += p.num2[i];
    }

    return sum;
}

// 構造体とメンバーの配列の複合
int struct8() {
    struct A {
        int num[2][2][2];
    };

    struct A p[2][2];

    int i0, i1, i2, i3, i4, i5;
    for (i0 = 0; i0 < 2; i0++) {
        for (i1 = 0; i1 < 2; i1++) {
            for (i2 = 0; i2 < 2; i2++) {
                for (i3 = 0; i3 < 2; i3++) {
                    for (i4 = 0; i4 < 2; i4++) {
                        p[i0][i1].num[i2][i3][i4] = i0 + i1 + i2 + i3 + i4;
                    }
                }
            }
        }
    }

    return p[1][1].num[0][1][0];
}

int struct9() {
    struct A {
        int num;
    } a, *b, c;
    b = malloc(sizeof(struct A));
    a.num = 1;
    b->num = 2;
    c.num = 3;
    return a.num + b->num + c.num;
}

int struct_arrow1() {
    struct A {
        int num;
    };
    struct A *p1, p0;
    p0.num = 1;
    p1 = &p0;
    return p1->num;
}

// 自分自身をメンバーに持つ構造体
int struct_arrow2() {
    struct A {
        int num;
        struct A *next;
    };

    struct A *p1 = 0;

    int i;
    for (i = 0; i < 5; i++) {
        struct A *p = malloc(12);
        p->next = p1;
        p->num = i + 1;
        p1 = p;
    }

    int sum = 0;
    while (p1) {
        sum += p1->num;
        p1 = p1->next;
    }
    return sum;
}

// ->が連続した場合
int struct_arrow3() {
    struct A {
        int num;
        struct A *next;
    };

    struct A *p1 = 0;

    int i;
    for (i = 0; i < 5; i++) {
        struct A *p = malloc(12);
        p->next = p1;
        p->num = i + 1;
        p1 = p;
    }

    return p1->next->next->next->next->num;
}

int struct_forward1() {
    struct A *a;
    struct A {
        int a;
        char b;
    };
    return sizeof(struct A);
}

typedef struct FORWARD FORWARD;
FORWARD *forward;
struct FORWARD {
    int member;
};
int struct_forward2() {
    forward = malloc(sizeof(FORWARD));
    forward->member = 10;
    return forward->member;
}

int struct_assign1() {
    struct A {
        int m1;
        int m2;
    } a, b;
    a.m1 = 10;
    a.m2 = 30;
    b = a;
    a.m1 = 40;
    return (b.m1 == 10) && (b.m2 == 30) && (a.m1 == 40);
}

int struct_assign2() {
    struct A {
        int m1;
        int m2;
    } a, *b, *c = malloc(sizeof(struct A));

    a.m1 = 10;
    a.m2 = 30;

    b = &a;
    *c = *b;
    return (c->m1 == 10 && c->m2 == 30);
}

int struct_assign3() {
    struct A {
        char m1[10];
        int m2;
    } a, b;
    for (int i = 0; i < 10; i++) a.m1[i] = i;
    a.m2 = 10;
    b = a;
    return b.m1[5] + b.m2;
}

int struct_assign4() {
    struct A {
        char m1[10];
        int m2;
    } a;
    struct B {
        struct A m1;
        char m2[10];
        int m3;
    } b, c;
    // a
    for (int i = 0; i < 10; i++) a.m1[i] = i;
    a.m2 = 10;
    // b
    b.m1 = a;
    for (int i = 0; i < 10; i++) b.m2[i] = 2 * i;
    b.m3 = 20;
    // c
    c = b;
    return c.m1.m1[5] + c.m2[5];
}

int main() {
    ASSERT(11, struct1(), "struct1");
    ASSERT(2, struct2(), "struct2");
    ASSERT(1, struct3(), "struct3");
    ASSERT(45, struct4(), "struct4");
    ASSERT(45, struct5(), "struct5");
    ASSERT(8, struct6(), "struct6");
    ASSERT(20, struct7(), "struct7");
    ASSERT(3, struct8(), "struct8");
    ASSERT(6, struct9(), "struct9");

    ASSERT(1, struct_arrow1(), "struct_arrow1");
    ASSERT(15, struct_arrow2(), "struct_arrow2");
    ASSERT(1, struct_arrow3(), "struct_arrow3");

    ASSERT(8, struct_forward1(), "struct_forward1");
    ASSERT(10, struct_forward2(), "struct_forward2");

    ASSERT(1, struct_assign1(), "struct_assign1");
    ASSERT(1, struct_assign2(), "struct_assign2");
    ASSERT(15, struct_assign3(), "struct_assign3");
    ASSERT(15, struct_assign4(), "struct_assign4");

    printf("ALL TEST OF struct.c SUCCESS :)\n");
    return 0;
}