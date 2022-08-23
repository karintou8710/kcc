#!/bin/bash

COMPILER=$1
# SRCのファイルを実行する
SRC=input.c
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

run() {
    if [ ! -e "$COMPILER" ]; then
        error "./$COMPILER does not exist"
        exit $FAILURE
    fi
    
    touch $SRC
    pre=`echo $SRC | sed -e 's/\.c/\.i/g'`
    cpp -I $DUMMY_LIB_DIR $SRC > $pre
    ./$COMPILER $pre > tmp.s
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
    status="$?"

    p "exit status => $status"
}

run