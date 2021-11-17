#!/bin/bash

for i in tests/test.c tests/global.c
do
    ./9cc $i > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp

    if [ $? = 1 ]; then
        exit 1
    fi

done
