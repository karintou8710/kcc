#!/bin/bash

KCC_FROM_GCC=$1
KCC_FROM_CLANG=$2
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

if [ ! -e "$KCC_FROM_GCC" ]; then
    error "./$KCC_FROM_GCC does not exist"
    exit $FAILURE
fi

if [ ! -e "$KCC_FROM_CLANG" ]; then
    error "./$KCC_FROM_CLANG does not exist"
    exit $FAILURE
fi

for src in tests/*.c
do
    p "$KCC_FROM_GCC start compileing $src"
    pre=`echo $src | sed -e 's/\.c/\.i/g'`
    cpp $src > $pre
    ./$KCC_FROM_GCC $pre > tmp_gcc.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        error "$KCC_FROM_GCC failed to compile"
        exit $FAILURE
    fi

	p "$KCC_FROM_CLANG start compileing $src"
    ASAN_OPTIONS=detect_leaks=0 ./$KCC_FROM_CLANG $pre > tmp_clang.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        error "$KCC_FROM_CLANG failed to compile"
        exit $FAILURE
    fi

	diff tmp_gcc.s tmp_clang.s
    ret=$?
    if [ $ret -ne $SUCCESS ]; then
        echo "diff tmp_gcc.s tmp_clang.s failed at $src"
        exit $FAILURE
    fi
done