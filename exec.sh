#!/bin/bash

exec() {
    ./9cc "`cat test_exec.c`" > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    echo "$input => $actual"
}

exec