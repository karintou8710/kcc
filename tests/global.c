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

int main() {

    ASSERT(3, globaltest1(), "globaltest1");
    ASSERT(2, globaltest2(), "globaltest2");

    printf("ALL TEST OF global.c SUCCESS :)\n");

    return 0;
}