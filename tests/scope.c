#include <stdio.h>
#include <string.h>

int ASSERT(int expected, int actual, char *name)
{
    if (expected == actual)
        return 0;

    printf("name:<%s> failed!!\n", name);
    printf("expected %d -> actual %d\n", expected, actual);
    exit(1);
}

int local_scope1()
{
    int a = 2;
    {
        a = 1;
        int a = 2;
        a = 3;
    }
    return a;
}

int local_scope2()
{
    int a = 2;
    {
        int a = 1;
        return a;
    }
    return a;
}

int local_scope3()
{
    int a = 1;
    {
        int a = 2;
        {
            int a = 3;
            {
                int a = 4;
            }
        }
        return a;
    }
}

int local_scope4()
{
    int res = 2;
    if (res == 2)
    {
        res = 3;
    }
    if (res == 3)
    {
        int res = 4;
    }
    if (res == 3)
    {
        int res = 5;
    }
    if (res == 3)
    {
        res = 6;
    }
    return res;
}

int local_scope5()
{
    int a = 1;
    {
        a = 2;
        {
            {
                {
                    {
                        {
                            {
                                int a = 3;
                            }
                        }
                    }
                }
            }
        }
        return a;
    }
}

int local_scope6()
{
    int i = 0, sum = 0;
    for (int i = 0; i < 10; i++)
    {
        sum += i;
        int i = 1;
        sum += 1;
    }
    return sum;
}

int local_scope7()
{
    int res = 0;
    for (int i = 0; i < 10; i++)
    {
        res += i;
    }
    for (int i = 0; i < 10; i++)
    {
        res += i;
    }
    return res;
}

int local_scope8()
{
    struct rgb
    {
        int r;
        int g;
        int b;
    };
    int res = 0;
    struct rgb p1;
    p1.r = 10;
    {
        struct rgb p1;
        p1.r = 30;
    }
    {
        struct rgb p1;
        p1.r = 50;
        {
            p1.r = 60;
        }
        res += p1.r;
    }
    res += p1.r;
}

int main()
{
    ASSERT(1, local_scope1(), "local_scope1");
    ASSERT(1, local_scope2(), "local_scope2");
    ASSERT(2, local_scope3(), "local_scope3");
    ASSERT(6, local_scope4(), "local_scope4");
    ASSERT(2, local_scope5(), "local_scope5");
    ASSERT(55, local_scope6(), "local_scope6");
    ASSERT(90, local_scope7(), "local_scope7");
    ASSERT(70, local_scope8(), "local_scope8");

    printf("ALL TEST OF scope.c SUCCESS :)\n");
    return 0;
}