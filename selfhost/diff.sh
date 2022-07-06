#!/bin/bash

EXIT_SUCCESS=0
EXIT_FAILURE=1

GEN2_ASM_FILES=`find . -path "./selfhost/gen2/*.s"`
for GAF2 in $GEN2_ASM_FILES; do
    GAF3=`echo $GAF2 | sed 's/gen2/gen3/g'`
    diff $GAF2 $GAF3
    ret=$?
    if [ $ret -ne $EXIT_SUCCESS ]; then
        echo "diff $GAF2 $GAF3 failure !"
        exit $EXIT_FAILURE
    fi
done

echo "success !!"
