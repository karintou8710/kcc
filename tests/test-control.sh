#!/bin/bash

COMPILER=$1
DUMMY_LIB_DIR=selfhost/dummy_headers
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

for src in tests/*.c
do
    debug "$COMPILER start compileing $src"
    pre=`echo $src | sed -e 's/\.c/\.i/g'`
    cpp $src > $pre
    ASAN_OPTIONS=detect_leaks=0 ./$COMPILER $pre > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$COMPILER failed to compile"
        exit $FAILURE
    fi

    cc -static -o tmp tmp.s
    ./tmp

    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$src failed to exec"
        exit $FAILURE
    fi

done