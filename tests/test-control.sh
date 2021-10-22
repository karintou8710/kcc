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
assert 6 'main() {a=6; return a;}'
assert 2 'main() {a=2; b=a; return b;}'
assert 3 'main() {a=1; b=2; c=a+b; return c;}'
assert 12 'main() {value=12; return value;}'
assert 6 'main() {val=1; t=3*(val+1); return t;}'
assert 5 'main() {num=1; test=(num*10)/2; return test;}'
assert 4 'main() {a=1; a=a+3; return a;}'
assert 5 'main() {a2=1; A_1=5; return a2*A_1;}'

# # 代入演算子
assert 2 'main() {a=1; a+=1; return a;}'
assert 4 'main() {a=10; a-=6; return a;}'
assert 6 'main() {a=2; a*=3; return a;}'
assert 10 'main() {a=20; a/=2; return a;}'
assert 10 'main() {a=5; b=a+=5; return b;}'

# # if else
assert 4 'main() {if (1) return 4;}'
assert 6 'main() {if (2+3==5) return 3*2;}'
assert 10 'main() {a=2*3; if (a==6) return a+4; else return 0;}'
assert 0 'main() {a=2*3; if (a<5) return a+4; else return 0;}'
assert 1 'main() {a=1;n=0; if(a==1){n=1;} if(a==2){n=2;} return n; }'

# # while
assert 10 'main() {i=0; while (i<10) i+=1; return i;}'

# # for
assert 10 'main() {a=0; for(i=0;i<10;i+=1) a=a+1; return a;}'

# # block
assert 1 'main() {return 1;}'
assert 2 'main() {a=1; if (a/2==0) {a*=2;return a;} else {return a;}}'
assert 20 'main() {a=0; for(i=1;i<=5;i+=1) {a+=i;a+=1;} return a;}'

# func call (関数の実態はリンクする)
assert 3 'main() {return ret();}'
assert 3 'main() {return ret(); }'
assert 5 'main() {a=1; return ret() + a*2;}'
assert 4 'main() {return constant(4);}'
assert 3 'main() {return add(1, 2);}'
assert 21 'main() {return add6(1, 2, 3, 4, 5, 6);}'
assert 66 'main() {return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 'main() {return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

# func define
assert 2 'test() {return 2;} main() {return test();}'
assert 5 'test() {return 4;} main() {a=1; return a+test();}'
assert 2 'test() {return 1;} main() {return test() + test();}'

assert 6 'test(a) {return a;} main() {return test(6);}'
assert 3 'add2(a,b) {return a+b;} main() {return add2(1,2);}'
assert 15 'test(a,b) {c=10;return c+a+b;} main() {return test(2, 3);}'
assert 8 'sum(n) {if (n==1) {return 1;} return sum(n-1) + sum(n-1);} main() {return sum(4);}'
assert 55 'fib(n) {if (n==1) {return 1;}if (n==2) {return 1;}return fib(n-1) + fib(n-2);}main() {return fib(10);}'
assert 3 'main() { x = 3;y = 5;z = &y + 8;return *z;} '

echo OK