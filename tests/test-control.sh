#!/bin/bash

for i in tests/test.c tests/global.c tests/loop.c tests/op.c
do
    ./9cc $i > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp

    if [ $? != 0 ]; then
        echo $i failed!!
        exit 1
    fi

done
