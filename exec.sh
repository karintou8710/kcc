#!/bin/bash

COMPILER="kcc"
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

exec() {
    if [ ! -e "$COMPILER" ]; then
        debug "./kcc does not exist"
        exit $FAILURE
    fi
    
    touch test_exec.c
    ./kcc test_exec.c > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "kcc failed to compile"
        exit $FAILURE
    fi
    
    cc -static -o tmp tmp.s
    ./tmp
    status="$?"

    debug "exit status => $status"
}

exec