#!/bin/bash

exec() {
    input="$1"

    ./9cc "$input" > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    echo "$input => $actual"
}

exec "$1"