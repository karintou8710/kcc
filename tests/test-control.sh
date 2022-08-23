#!/bin/bash

COMPILER=$1
DUMMY_LIB_DIR=selfhost/dummy_headers
SUCCESS=0
FAILURE=1

p () {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

error() {
    # red color
    p "\033[31m$1\033[m"
}

success() {
    # green color
    p "\033[32m$1\033[m"
}

if [ ! -e "$COMPILER" ]; then
    error "./$COMPILER does not exist"
    exit $FAILURE
fi

for src in tests/*.c
do
    p "$COMPILER start compileing $src"
    pre=`echo $src | sed -e 's/\.c/\.i/g'`
    cpp $src > $pre
    ASAN_OPTIONS=detect_leaks=0 ./$COMPILER $pre > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        error "$COMPILER failed to compile"
        exit $FAILURE
    fi

    cc -o tmp tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        error "$src failed to assemble"
        exit $FAILURE
    fi

    ./tmp

    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        error "$src failed to exec"
        exit $FAILURE
    fi

done


success "test-control.sh success!"