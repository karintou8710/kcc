#include <stdio.h>

int ASSERT(int expected, int actual, char *name)
{
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int globaltest1_a;
int globaltest1_glo() { globaltest1_a = 2; }
int globaltest1()
{
    int globaltest1_a;
    globaltest1_a = 1;
    int b;
    b = globaltest1_glo();
    return globaltest1_a + b;
}

int globaltest2_arr[3][3];
int globaltest2_glo()
{
    int i;
    int j;
    for (i = 0; i < 3; i += 1)
    {
        for (j = 0; j < 3; j += 1)
        {
            globaltest2_arr[i][j] = i + j;
        }
    }
}
int globaltest2()
{
    globaltest2_glo();
    return globaltest2_arr[1][1];
}

struct A {
	int num;
	int num2;
	int num3;
};
struct A glo, glo2, glo3;
int globalstruct1()
{
    glo.num = 1;
	glo.num2 = 10;
	glo.num3 = 100;
	return glo.num + glo.num2 + glo.num3;
}

int globalstruct2()
{
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
struct Vector
{
    void **body;
    int len;
    int capacity;
};
struct Vector *new_vec()
{
    struct Vector *v = malloc(sizeof(struct Vector));
    v->body = malloc(16 * sizeof(void *));
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(struct Vector *v, void *elem)
{
    if (v->len == v->capacity)
    {
        v->capacity *= 2;
        v->body = realloc(v->body, sizeof(void *) * v->capacity);
    }
    v->body[v->len++] = elem;
}
int globalstruct3()
{
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

int globalstruct4()
{
    p0.b[1][0] = 1;
    p1.b[0][1] = 1;
    p1.p = &p0;
    return p1.b[0][1] + p1.p->b[1][0];
}

struct B *_return_structB()
{
    struct B *p = malloc(sizeof(struct B*));
    p->a = 10;
    p->b[0][1] = 5;
    return p;
}
int globalstruct5()
{
    struct B *p = _return_structB();
    p->b[1][0] = 4;
    return p->a + p->b[0][1] + p->b[1][0];
}

struct C
{
    struct A pa[4];
    struct B pb;
    int num;
};

struct C gloc;
int globalstruct6()
{
    gloc.num = 10;
    gloc.pa[1].num = 20;
    gloc.pb.a = 20;
    return gloc.num + gloc.pa[1].num + gloc.pb.a;
}

int main() {

    ASSERT(3, globaltest1(), "globaltest1");
    ASSERT(2, globaltest2(), "globaltest2");

    ASSERT(111, globalstruct1(), "globalstruct1");
    ASSERT(122, globalstruct2(), "globalstruct2");
    ASSERT(10, globalstruct3(), "globalstruct3");
    ASSERT(2, globalstruct4(), "globalstruct4");
    ASSERT(19, globalstruct5(), "globalstruct5");
    ASSERT(50, globalstruct6(), "globalstruct6");

    printf("ALL TEST OF global.c SUCCESS :)\n");

    return 0;
}