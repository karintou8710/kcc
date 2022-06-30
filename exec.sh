#!/bin/bash

COMPILER="kcc2"
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
    
    touch test_exec.c
    ./$COMPILER test_exec.c > tmp.s
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