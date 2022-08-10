#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int *_return_type_cast1_sub() {
    int *a = malloc(4);
    *a = 10;
    return a;
}

int return_type_cast1() {
    int *a = _return_type_cast1_sub();
    *a += 5;
    return *a;
}

void _return_type_cast2_sub() {
    int a = 1;
    int b = 2;
    return;
}

// 実行できるかテスト
int return_type_cast2() {
    _return_type_cast2_sub();
    return 0;
}

char _return_type_cast3_sub() {
    return 257;  // 1,00000001
}

int return_type_cast3() {
    int b = _return_type_cast3_sub();
    return b;
}

char assign_type_cast1() {
    char a = 257;
    return a;
}

int assign_type_cast2() {
    char a = 127;
    int b = a;
    return b;
}

int assign_type_cast3() {
    int a = 257;
    char b = a;
    return b;
}

int *assign_type_cast4() {
    char a = 1;
    int *b = a;
    return b;
}

int pointer_sub1() {
    int a = 1;
    int *p1 = &a, *p2 = &a;
    p2 += 3;
    return p2 - p1;
}

int pointer_sub2() {
    int a[100];
    return a + (3) - a;
}

int pointer_sub3() {
    int a[100];
    int *b = a + 10;
    return b + 1 - a;
}

int pointer_sub4() {
    int a[2] = {1, 2};
    return 2 + *((a + 2) - 1);
}

long long1() {
    long a = 10;
    a += 1000000000000000;
    return a;
}

long long2() {
    long a = 100000000000127;
    return (long)(char)a;
}

long long long3() {
    long long int a = 100000000000000;
    long long b = 100000000000000;
    long int c = 100000000000000;
    return a + b + c;
}

short short1() {
    short a = (1 << 14) + (1 << 20);
    return a;
}

short short2() {
    short a = (1 << 14) + 127;
    return (short)(char)a;
}

int cast5() {
    int a[10];
    int *b = (int *)a;
    *b = 10;
    return a[0];
}

int cast6() {
    int a[2];
    struct A {
        int m1;
        int m2;
    };
    a[0] = 100;
    a[1] = 200;
    struct A c = *(struct A *)a;
    return (c.m1 == 100) && (c.m2 == 200);
}

int params1(int a[3]) {
    return a[0];
}

typedef _Bool bool;
bool true = 1;
bool false = 0;

bool bool1() {
    bool a = false, b = true;
    a = 1000 + 1;
    b = 10;

    return a + b;
}

bool bool2() {
    int a = 10;
    return (1 * 2 - 2) * a;
}

int bool3(bool a) {
    return a;
}

int bool4() {
    bool a[3] = {1, 2};
    return a[0] == true && a[1] == true && a[2] == false;
}

int bool5() {
    struct A {
        bool test;
    } a;
    a.test = -1;
    return a.test;
}

signed int signed1(unsigned int a, int signed b) {
    long signed int long c = 10;
    return a + b + c;
}

int nest = 10;
int((*nest2)) = &nest;

int nested_type1() {
    int(*a)[5][10];
    int b[5][10];
    a = &b;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            b[i][j] = i + j;
        }
    }

    return (*a)[1][2];
}

int nested_type2() {
    int(*a[10])[20];
    return sizeof(a);
}

int nested_type3() {
    int(*(*(*(*a))[10]))[3];
    return sizeof(a);
}

int nested_type4() {
    int a = 1;
    int(*(b)) = &a;
    return *b;
}

int nested_type5(int (*a)[2]) {
    return (*a)[0] + (*a)[1];
}

int func_pointer1() {
    int (*a)();
    a = nested_type1;
    int res = a();
    return res;
}

int _test_func2(int a, int b) {
    return a + b;
}

int func_pointer2() {
    int (*a)(int, int) = _test_func2;
    int (**b)(int, int) = &a;
    int res = (*b)(1, 2);
    return res;
}

bool _test_func3(bool a) {
    return a + 10;
}

int func_pointer3() {
    bool (*a)(bool a) = _test_func3;
    int res = a(false);
    return res;
}

int _test_func4_1(int (*a)(int p1, long p2)) {
    int res = a(1, 2);
    return res;
}

int _test_func4_2(int p1, long p2) {
    return p1 + p2;
}

int func_pointer4() {
    int res = _test_func4_1(_test_func4_2);
    return res;
}

char *_test_func5(char *fmt, ...) {
    char buf[100];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    return buf;
}

int func_pointer5() {
    char *(*p)(char *fmt, ...) = _test_func5;
    int a = 10;
    return strcmp("t10", p("t%d", a)) == 0;
}

int (*_test_func6())(int, long) {
    return _test_func4_2;
}

int func_pointer6() {
    return _test_func6()(1, 2);
}

// 配列を戻り値とするのは未対応
int main() {
    ASSERT(15, return_type_cast1(), "return_type_cast1");
    ASSERT(0, return_type_cast2(), "return_type_cast2");
    ASSERT(1, return_type_cast3(), "return_type_cast3");
    ASSERT(1, assign_type_cast1(), "assign_type_cast1");
    ASSERT(127, assign_type_cast2(), "assign_type_cast2");
    ASSERT(1, assign_type_cast3(), "assign_type_cast3");
    ASSERT(1, assign_type_cast4(), "assign_type_cast4");
    ASSERT(3, pointer_sub1(), "pointer_sub1");
    ASSERT(3, pointer_sub2(), "pointer_sub2");
    ASSERT(11, pointer_sub3(), "pointer_sub3");
    ASSERT(4, pointer_sub4(), "pointer_sub4");

    ASSERT(0, (char)(256), "cast1");
    ASSERT(4, (char)((1 << 30) + (1 << 2)), "cast2");
    ASSERT(0, (int)(char)(char *)(256), "cast3");
    ASSERT(100, (char)(256 + 10 * 10), "cast4");
    ASSERT(10, cast5(), "cast5");
    ASSERT(1, cast6(), "cast6");
    ASSERT(1, (_Bool)1, "cast7");
    ASSERT(1, (_Bool)1234, "cast8");
    ASSERT(0, (_Bool)(int *)(_Bool)0, "cast9");

    ASSERT(8, sizeof(long int long), "sizeof(long int long)");
    ASSERT(8, sizeof(long long), "sizeof(long long)");
    ASSERT(8, sizeof(long), "sizeof(long)");
    ASSERT(8, sizeof(int(*(*)[20])[10]), "sizeof(int (*(*)[20])[10])");
    ASSERT(80, sizeof(_Bool(*[10])[]), "sizeof(_Bool(*[10])[])");
    ASSERT(10, sizeof("123456789"), "sizeof(\"123456789\")");

    ASSERT(1000000000000010, long1(), "long1");
    ASSERT(127, long2(), "long2");
    ASSERT(300000000000000, long3(), "long3");

    ASSERT(1 << 14, short1(), "short1");
    ASSERT(127, short2(), "short2");

    int test1[3] = {1, 2, 3};
    int *test2 = test1;
    int test3 = 10;
    int *test4 = &test3;
    ASSERT(1, params1(test2), "params1(test2)");
    ASSERT(10, params1(test4), "params1(test4)");

    ASSERT(1, bool1(), "bool1()");
    ASSERT(0, bool2(), "bool2()");
    ASSERT(1, bool3(10), "bool3(10)");
    ASSERT(0, bool3(0), "bool3(0)");
    ASSERT(1, bool4(), "bool4()");
    ASSERT(1, bool5(), "bool5()");

    ASSERT(12, signed1(1, 1), "signed1(1,1)");

    ASSERT(10, *nest2, "*nest2");

    ASSERT(3, nested_type1(), "nested_type1()");
    ASSERT(80, nested_type2(), "nested_type2()");
    ASSERT(8, nested_type3(), "nested_type3()");
    ASSERT(1, nested_type4(), "nested_type4()");
    int a[2] = {1, 2};
    ASSERT(3, nested_type5(&a), "nested_type5(&a)");

    ASSERT(3, func_pointer1(), "func_pointer1()");
    ASSERT(3, func_pointer2(), "func_pointer2()");
    ASSERT(1, func_pointer3(), "func_pointer3()");
    ASSERT(3, func_pointer4(), "func_pointer4()");
    ASSERT(1, func_pointer5(), "func_pointer5()");
    ASSERT(3, func_pointer6(), "func_pointer6()");

    printf("ALL TEST OF type.c SUCCESS :)\n");
    return 0;
}