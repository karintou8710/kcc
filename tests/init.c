#include "basic.h"

int ASSERT(int expected, int actual, char *name) {
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int init_array1() {
    int a[2][2] = {
        {1, 2},
        {3, 4}};
    return a[0][0] + a[1][0];
}

int init_array2() {
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    return a[1] + a[9];
}

int init_array3() {
    char a[2][5] = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10}};
    return a[0][1] + a[1][4];
}

int init_array4() {
    int a[1][1][1][1][1][1][1][1] = {{{{{{{{1}}}}}}}};
    return a[0][0][0][0][0][0][0][0];
}

int init_array5() {
    int a[2][2] = {
        {init_array1() || 1, init_array2() - 1},
        {init_array3() + 3, init_array4() * 2}};
    return a[0][1] + a[1][1];
}

int init_array6() {
    int a[10] = {1, 0, 112};
    return a[2] + a[4] + a[5] + a[9];
}

int init_array7() {
    int a[2][2][2] = {
        {{1},
         {2}},
        {{}, {3}}};
    return a[0][0][0] + a[0][0][1] + a[1][1][0];
}

int init_array8() {
    int a[] = {1, 2, 3, 4, 5};
    return a[0] + a[4];
}

int init_array9() {
    int a[][10] = {
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {1, 2},
        {3, 4}};
    return a[7][0] + a[0][1];
}

int init_array10() {
    char a[10] = "abcdefg";
    char b[] = "abcdefg";
    return a[0] + a[7] + b[1] + b[7];
}

int init_array11() {
    char a[][10] = {
        "hello",
        "world",
        "test"};
    return a[0][0] + a[1][1] + a[2][2];
}

int init_array12() {
    char *a[5] = {
        "hello",
        "world",
        "test"};
    return a[0][0] + a[1][1] + a[2][2];
}

int init_struct1() {
    struct A {
        int member;
        int member2;
    };
    struct A a = {0, 1};

    return a.member + a.member2;
}

int init_struct2() {
    struct A {
        int member;
        int member2;
    };
    struct A a = {}, b = {1}, c = {1 * 2, 3 - 4};

    return a.member2 == 0 && b.member2 == 0 && c.member2 == -1;
}

int init_struct3() {
    struct A {
        char a;
        short b;
    } a = {
        127,
        127};
    return a.a == 127 && a.b == 127;
}

int init_struct4() {
    struct A {
        char *str;
    };
    struct A a = {
        "test"};
    return strcmp("test", a.str) == 0;
}

int init_struct5() {
    struct A {
        int a;
        int *b;
        char *str;
    };
    struct A a = {};
    return a.str == 0;
}

int init_struct6() {
    struct A {
        int member[4];
    };
    struct A a = {
        {1, 2, 3}};

    return a.member[0] == 1 && a.member[1] == 2 && a.member[3] == 0;
}

int init_struct7() {
    struct A {
        int a;
        int member[4];
    } a[2] = {
        {1, {1, 2, 3, 4}},
        {2, {1 * 2, 2 * 2, 3 * 2, 4 * 2}}};

    return a[0].a == 1 && a[0].member[2] == 3 && a[1].a == 2 && a[1].member[3] == 8;
}

int init_struct8() {
    struct A {
        int member;
    };
    struct B {
        struct A a_struct[2];
    } b[2] = {
        {{{1},
          {2}}},
        {{{3},
          {4}}}};
    return b[0].a_struct[0].member == 1 && b[1].a_struct[1].member == 4;
}

int test1 = 10;
int test2_1 = 1 + 2 - (3 * 4) / 5, test2_2 = 1;
int *test3_1 = 1, test3_2 = 2;
char test4 = 127;
char test5 = 'a';
char test6 = sizeof(test3_1);
int test7[3] = {1, 2, 3};
int test8[10] = {1};
int test9[] = {1, 2, 3};
int test10[][2] = {
    {1, 2},
    {3, 4},
    {5, 6}};
char test11[10] = "bac";
char *test12 = "bac" + 1;
int *test13 = test7 + 1;
char *test14 = &test4;
int test15 = (1 && 0) && !(0 && 0);
int test16 = (1 < 2) && (1 <= 2);
int test17 = (1 == 2) && (1 != 2);

int main() {
    ASSERT(4, init_array1(), "init_array1");
    ASSERT(12, init_array2(), "init_array2");
    ASSERT(12, init_array3(), "init_array3");
    ASSERT(1, init_array4(), "init_array4");
    ASSERT(13, init_array5(), "init_array5");
    ASSERT(112, init_array6(), "init_array6");
    ASSERT(4, init_array7(), "init_array7");
    ASSERT(6, init_array8(), "init_array8");
    ASSERT(5, init_array9(), "init_array9");
    ASSERT(195, init_array10(), "init_array10");
    ASSERT(330, init_array11(), "init_array11");
    ASSERT(330, init_array12(), "init_array12");

    ASSERT(10, test1, "test1");
    ASSERT(1, test2_1, "test2_1");
    ASSERT(1, test2_2, "test2_2");
    ASSERT(8, sizeof(test3_1), "sizeof(test3_1)");
    ASSERT(4, sizeof(test3_2), "sizeof(test3_2)");
    ASSERT(127, test4, "test4");
    ASSERT(97, test5, "test5");
    ASSERT(8, test6, "test6");
    ASSERT(2, test7[1], "test7[1]");
    ASSERT(0, test8[9], "test8[9]");
    ASSERT(2, test9[1], "test9[1]");
    ASSERT(7, test10[0][1] + test10[2][0], "test10[0][1] + test10[2][0]");
    ASSERT(97, test11[1], "test11[1]");
    ASSERT(97, test12[0], "test12[0]");
    ASSERT(2, test13[0], "test13[0]");
    ASSERT(127, *test14, "*test14");
    ASSERT(0, test15, "test15");
    ASSERT(1, test16, "test16");
    ASSERT(0, test17, "test17");

    ASSERT(1, init_struct1(), "init_struct1");
    ASSERT(1, init_struct2(), "init_struct2");
    ASSERT(1, init_struct3(), "init_struct3");
    ASSERT(1, init_struct4(), "init_struct4");
    ASSERT(1, init_struct5(), "init_struct5");
    ASSERT(1, init_struct6(), "init_struct6");
    ASSERT(1, init_struct7(), "init_struct7");
    ASSERT(1, init_struct8(), "init_struct8");

    printf("ALL TEST OF init.c SUCCESS :)\n");
    return 0;
}