#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int globaltest1_a;
int globaltest1_glo() { globaltest1_a = 2; }
int globaltest1() {
    int globaltest1_a;
    globaltest1_a = 1;
    int b;
    b = globaltest1_glo();
    return globaltest1_a + b;
}

int globaltest2_arr[3][3];
int globaltest2_glo() {
    int i;
    int j;
    for (i = 0; i < 3; i += 1) {
        for (j = 0; j < 3; j += 1) {
            globaltest2_arr[i][j] = i + j;
        }
    }
}
int globaltest2() {
    globaltest2_glo();
    return globaltest2_arr[1][1];
}

int *s1, s2, **s3, s4;
int globaltest3() {
    return sizeof(s1) + sizeof(s2) + sizeof s3 + sizeof s4;
}

struct A {
    int num;
    int num2;
    int num3;
};
struct A glo, glo2, glo3;
int globalstruct1() {
    glo.num = 1;
    glo.num2 = 10;
    glo.num3 = 100;
    return glo.num + glo.num2 + glo.num3;
}

int globalstruct2() {
    struct A {
        int num4;
        int num5;
        struct A *p;
    };
    struct A p;
    p.num4 = 10;
    p.num5 = 1;
    return glo.num + glo.num2 + glo.num3 + p.num4 + p.num5;
}

// vector test
struct Vector {
    void **body;
    int len;
    int capacity;
};
struct Vector *new_vec() {
    struct Vector *v = malloc(sizeof(struct Vector));
    v->body = malloc(16 * sizeof(void *));
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(struct Vector *v, void *elem) {
    if (v->len == v->capacity) {
        v->capacity *= 2;
        v->body = realloc(v->body, sizeof(void *) * v->capacity);
    }
    v->body[v->len++] = elem;
}
int globalstruct3() {
    struct Vector *v = new_vec();
    int a = 10;
    vec_push(v, &a);
    int *b = v->body[0];
    return *b;
}

struct B {
    int a;
    int b[2][2];
    struct B *p;
};
struct B p0, p1;

int globalstruct4() {
    p0.b[1][0] = 1;
    p1.b[0][1] = 1;
    p1.p = &p0;
    return p1.b[0][1] + p1.p->b[1][0];
}

struct B *_return_structB() {
    struct B *p = malloc(sizeof(struct B *));
    p->a = 10;
    p->b[0][1] = 5;
    return p;
}
int globalstruct5() {
    struct B *p = _return_structB();
    p->b[1][0] = 4;
    return p->a + p->b[0][1] + p->b[1][0];
}

struct C {
    struct A pa[4];
    struct B pb;
    int num;
};

struct C gloc;
int globalstruct6() {
    gloc.num = 10;
    gloc.pa[1].num = 20;
    gloc.pb.a = 20;
    return gloc.num + gloc.pa[1].num + gloc.pb.a;
}

struct D {
    int a;
    int b;
    int c;
} * *b1, *b2, b3;
int globalstruct7() {
    b3.a = 1;
    b2 = &b3;
    b1 = &b2;
    (**b1).a = 2;
    return b3.a;
}

enum enumA {
    A = 10,
    B,
    C = -1,
};

int global_enum1(int index) {
    enum enumA {
        A = 1,
        B,
        C
    };
    if (index == 0)
        return A;
    else if (index == 1)
        return B;
    else if (index == 2)
        return C;
}

int _global_func_pointer1(int a, int b) {
    return a + b;
}

int _global_func_pointer2(int a, int b) {
    return a - b;
}

int (*f)(int, int) = _global_func_pointer1;

int global_func_pointer1() {
    int t = f(1, 2);
    return t;
}

int (*f2[2])(int, int) = {
    _global_func_pointer1,
    _global_func_pointer2};

int global_func_pointer2() {
    int t = f2[0](1, 2);
    int t2 = f2[1](1, 2);
    return t == 3 && t2 == -1;
}

int main() {
    ASSERT(3, globaltest1(), "globaltest1");
    ASSERT(2, globaltest2(), "globaltest2");
    ASSERT(24, globaltest3(), "globaltest3");

    ASSERT(111, globalstruct1(), "globalstruct1");
    ASSERT(122, globalstruct2(), "globalstruct2");
    ASSERT(10, globalstruct3(), "globalstruct3");
    ASSERT(2, globalstruct4(), "globalstruct4");
    ASSERT(19, globalstruct5(), "globalstruct5");
    ASSERT(50, globalstruct6(), "globalstruct6");
    ASSERT(2, globalstruct7(), "globalstruct7");

    ASSERT(10, A, "enumA A");
    ASSERT(11, B, "enumA B");
    ASSERT(-1, C, "enumA C");

    ASSERT(1, global_enum1(0), "global_enum1(0)");
    ASSERT(2, global_enum1(1), "global_enum1(1)");
    ASSERT(3, global_enum1(2), "global_enum1(2)");

    ASSERT(3, global_func_pointer1(), "global_func_pointer1");
    ASSERT(1, global_func_pointer2(), "global_func_pointer2");

    printf("ALL TEST OF global.c SUCCESS :)\n");

    return 0;
}