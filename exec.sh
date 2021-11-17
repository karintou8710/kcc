#!/bin/bash

exec() {
    ./9cc test_exec.c > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp
    status="$?"

    echo "exit status => $status"
}

exec