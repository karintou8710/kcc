#!/bin/bash

COMPILER=$1
# SRCのファイルを実行する
SRC=test_exec.c
DUMMY_LIB_DIR=selfhost/dummy_headers
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

run() {
    if [ ! -e "$COMPILER" ]; then
        debug "./$COMPILER does not exist"
        exit $FAILURE
    fi
    
    touch $SRC
    pre=`echo $SRC | sed -e 's/\.c/\.i/g'`
    cpp -I $DUMMY_LIB_DIR $SRC > $pre
    ./$COMPILER $pre > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "$COMPILER failed to compile"
        exit $FAILURE
    fi
    
    cc -static -o tmp tmp.s
    ./tmp
    status="$?"

    debug "exit status => $status"
}

run