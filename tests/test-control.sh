#!/bin/bash

# 引数はexpected, inputの順 
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# ローカル変数
assert 6 'int main() {int a; a=6; return a;}'
assert 2 'int main() {int a; int b; a=2; b=a; return b;}'
assert 3 'int main() {int a; int b; int c; a=1; b=2; c=a+b; return c;}'
assert 12 'int main() {int value; value=12; return value;}'
assert 6 'int main() {int val; int t; val=1; t=3*(val+1); return t;}'
assert 5 'int main() {int num; int test; num=1; test=(num*10)/2; return test;}'
assert 4 'int main() {int a;a=1; a=a+3; return a;}'
assert 5 'int main() {int a2; int A_1; a2=1; A_1=5; return a2*A_1;}'

# 代入演算子
assert 2 'int main() {int a; a=1; a+=1; return a;}'
assert 4 'int main() {int a; a=10; a-=6; return a;}'
assert 6 'int main() {int a; a=2; a*=3; return a;}'
assert 10 'int main() {int a; a=20; a/=2; return a;}'
assert 10 'int main() {int a; int b; a=5; b=a+=5; return b;}'

# if else
assert 4 'int main() {if (1) return 4;}'
assert 6 'int main() {if (2+3==5) return 3*2;}'
assert 10 'int main() {int a; a=2*3; if (a==6) return a+4; else return 0;}'
assert 0 'int main() {int a; a=2*3; if (a<5) return a+4; else return 0;}'
assert 1 'int main() {int a; int n; a=1; n=0; if(a==1){n=1;} if(a==2){n=2;} return n; }'
assert 3 'int main() {int sum;sum=1;if(sum%2!=0)
        {sum+=1;if(sum%2==0){sum+=1;}else{sum=0;}}else{sum = 0;}return sum;}'
assert 30 'int main() {int a;a=10;if (a==10) {a=20;} if (a==20) {a=30;} return a;}'

# while
assert 10 'int main() {int i; i=0; while (i<10) i+=1; return i;}'
assert 12 'int main() {int i;i=0;int j;while (i<10) 
        { j=0;while (j<3) {i+=1;j+=1;} } return i;}'
assert 10 'int main() {int i;int sum;sum=0;i=0;while(i<5){sum+=1;i+=1;}
        while(i<10){sum+=1;i+=1;} return sum;}'

# for
assert 10 'int main() {int a; int i; a=0; for(i=0;i<10;i+=1) a=a+1; return a;}'
assert 6 'int main() {int i;int sum;sum=0;for(i=0;i<3;i+=1)
        {sum+= 1;}for(i=0;i<3;i+=1){sum += 1;}return sum;}'
assert 9 'int main() {int sum;int i;int j;for (i=0;i<3;i+=1) 
        {for (j=0;j<3;j+=1) {sum += 1;}} return sum;}'

# for-while
assert 5 'int main() {int i;int sum;sum=0;for(i=0;i<3;i+=1)
        {sum+= 1;}while(i<5){sum+=1;i+=1;} return sum;}'

# block
assert 1 'int main() {return 1;}'
assert 2 'int main() {int a; a=1; if (a/2==0) {a*=2;return a;} else {return a;}}'
assert 20 'int main() {int a; int i; a=0; for(i=1;i<=5;i+=1) {a+=i;a+=1;} return a;}'

# null statement
assert 1 'int main() {;;;;;;return 1;}'


# func call (関数の実態はリンクする)
assert 3 'int main() {return ret();}'
assert 3 'int main() {return ret(); }'
assert 5 'int main() {int a; a=1; return ret() + a*2;}'
assert 4 'int main() {return constant(4);}'
assert 3 'int main() {return add(1, 2);}'
assert 21 'int main() {return add6(1, 2, 3, 4, 5, 6);}'
assert 66 'int main() {return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 'int main() {return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

# func define
assert 2 'int test() {return 2;} int main() {return test();}'
assert 5 'int test() {return 4;} int main() {int a;a=1; return a+test();}'
assert 2 'int test() {return 1;} int main() {return test() + test();}'

assert 6 'int test(int a) {return a;} int main() {return test(6);}'
assert 3 'int add2(int a,int b) {return a+b;} int main() {return add2(1,2);}'
assert 15 'int test(int a, int b) {int c;c=10;return c+a+b;} int main() {return test(2, 3);}'
assert 8 'int sum(int n) {if (n==1) {return 1;} return sum(n-1) + sum(n-1);} int main() {return sum(4);}'
assert 55 'int fib(int n) {if (n==1) {return 1;}if (n==2) {return 1;} return fib(n-1) + fib(n-2);}int main() {return fib(10);}'


# ポインター
assert 3 'int main() {int x;int *y;y = &x;*y = 3;return x;}'
assert 10 'int main() {int a;int *b;int **c;int ***d;d = &c;c = &b;b = &a;a = 10;return ***d;}'
assert 2 'int main() {int *p;alloc4(&p, 1, 2, 4, 8);int *q;q = p + 1;return *q;}'
assert 4 'int main() {int *p;alloc4(&p, 1, 2, 4, 8);int *q;q = p + 3; q = q - 1;return *q;}'
assert 10 'int ptr(int *a) {*a = 10;return 0;} int main() {int b;b = 1;ptr(&b);return b;}'

# sizeof
assert 4 'int main() {int x;return sizeof x;}'
assert 4 'int main() {int x;return sizeof(x);}'
assert 12 'int main() {int x;int *y;return sizeof(x) + sizeof(y);}'
assert 4 'int main() {int *x;return sizeof(*x);}'
assert 20 'int main() {int a[5]; return sizeof(a);}'

# 配列
assert 3 'int main() {int a[2];*a = 1;*(a + 1) = 2;int *p;p = a;return *p + *(p + 1);}'
assert 6 'int main() {int a[3];*(a)=1;*(a+1)=2;*(a+2)=3;int i;int sum;
        for (i=0;i<3;i+=1) {sum += *(a+i);} return sum;}'
assert 2 'int main() {int a;int b[2];a=1;b[1]=a;return b[1] * 2;}'
assert 4 'int main() {int a[2];int b[2];int c;c=1;a[0]=c;a[1]=a[0]+1;
        b[0]=a[1]+1;b[1]=b[0]+1;return b[1];}'
assert 5 'int main() {int a[3][3];int i;int j;for (i=0;i<3;i+=1) 
        {for (j=0;j<3;j+=1) {a[i][j]=2*i+j;}} return a[2][1];}'

# MOD
assert 30 'int gcd(int a,int b){if(b==0){return a;}return gcd(b,a%b);}int main(){return gcd(630,300);}'
assert 1 'int main(){int a;int b;int c;a=100;b=21;c=3;return a%b%c;}'

# global
assert 3 'int a;int glo() {a = 2;}int main() {int a;a=1;int b;b = glo(); a+b;}'
assert 2 'int arr[3][3];int glo() {int i;int j;for (i=0;i<3;i+=1) {for (j=0;j<3;j+=1) {arr[i][j] = i+j;}}}int main() {glo(); return arr[1][1];}'

# char
assert 3 'int main() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;return x[0] + y;}'

# string_literal
assert 0 'int main() {char *a;a = "Hello, C-compiler\n";printf("%s\n", a);return 0;}'

echo OK