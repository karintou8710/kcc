#!/bin/bash

COMPILER="9cc"
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

if [ ! -e "$COMPILER" ]; then
    debug "./9cc does not exist"
    exit $FAILURE
fi

for i in tests/test.c tests/global.c tests/loop.c tests/op.c tests/type.c tests/struct.c tests/scope.c
do
    debug "9cc start compileing $i"
    ./9cc $i > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "9cc failed to compile"
        exit $FAILURE
    fi

    cc -static -o tmp tmp.s tmp2.o
    ./tmp

    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$i failed to exec"
        exit $FAILURE
    fi

done