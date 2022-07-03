#!/bin/bash

COMPILER=$1
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

if [ ! -e "$COMPILER" ]; then
    debug "./$COMPILER does not exist"
    exit $FAILURE
fi

for i in tests/*.c
do
    debug "$COMPILER start compileing $i"
    ASAN_OPTIONS=detect_leaks=0 ./$COMPILER $i > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$COMPILER failed to compile"
        exit $FAILURE
    fi

    cc -static -o tmp tmp.s
    ./tmp

    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$i failed to exec"
        exit $FAILURE
    fi

done