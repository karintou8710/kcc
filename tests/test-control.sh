#!/bin/bash

COMPILER="kcc"
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

if [ ! -e "$COMPILER" ]; then
    debug "./kcc does not exist"
    exit $FAILURE
fi

for i in tests/*.c
do
    debug "kcc start compileing $i"
    ./kcc $i > tmp.s
    ERRCHK=$?
    if [ $ERRCHK -ne $SUCCESS ]; then
        debug "kcc failed to compile"
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