#!/bin/bash

KCC_FROM_GCC=$1
KCC_FROM_CLANG=$2
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

if [ ! -e "$KCC_FROM_GCC" ]; then
    debug "./$KCC_FROM_GCC does not exist"
    exit $FAILURE
fi

if [ ! -e "$KCC_FROM_CLANG" ]; then
    debug "./$KCC_FROM_CLANG does not exist"
    exit $FAILURE
fi

for i in tests/*.c
do
    debug "$KCC_FROM_GCC start compileing $i"
    ./$KCC_FROM_GCC $i > tmp_gcc.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$KCC_FROM_GCC failed to compile"
        exit $FAILURE
    fi

	debug "$KCC_FROM_CLANG start compileing $i"
    ASAN_OPTIONS=detect_leaks=0 ./$KCC_FROM_CLANG $i > tmp_clang.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$KCC_FROM_CLANG failed to compile"
        exit $FAILURE
    fi

	diff tmp_gcc.s tmp_clang.s
    ret=$?
    if [ $ret -ne $SUCCESS ]; then
        echo "diff tmp_gcc.s tmp_clang.s failed at $i"
        exit $FAILURE
    fi
done