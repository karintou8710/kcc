int main() {
    int n; int i; int a[1000];
    n = 1000;
    for (i=0;i<n;i+=1) { a[i] = 1; }
    a[0] = 0; a[1] = 0;
    int j;
    for (i=2;i<n;i+=1) {
        if (a[i] != 0) {
            for (j=2*i;j<n;j+=i) {
                a[j] = 0;
            }
        }
    }
    
    for (i=2;i<n;i+=1) {
        if (a[i] == 1) {
            int_print_cumma(i);
        }
    }
    new_line();

    return 0;
}