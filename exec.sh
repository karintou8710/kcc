#!/bin/bash

COMPILER="9cc"
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

exec() {
    if [ ! -e "$COMPILER" ]; then
        debug "./9cc does not exist"
        exit $FAILURE
    fi

    ./9cc test_exec.c > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "9cc failed to compile"
        exit $FAILURE
    fi
    
    cc -static -o tmp tmp.s
    ./tmp
    status="$?"

    debug "exit status => $status"
}

exec