#!/bin/bash

# 引数はexpected, inputの順 
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
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
assert 6 'a=6;'
assert 2 'a=2;b=a;b;'
assert 3 'a=1;b=2;c=a+b;'
assert 12 'value=12;'
assert 6 'val=1;t=3*(val+1);t;'
assert 5 'num=1;test=(num*10)/2;test;'

# return
assert 5 'return 5;'
assert 6 'a=6;return a;'
assert 10 'a=1;b=4;return 2*(a+b);'
assert 13 'val=4;return val-2+11;'

# if else
assert 4 'if (1) 4;'
assert 6 'if (2+3==5) 3*2;'
assert 10 'a=2*3;if (a==6) return a+4; else return 0;'
assert 0 'a=2*3;if (a<5) return a+4; else return 0;'

echo OK