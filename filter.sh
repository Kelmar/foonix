#!/bin/bash

# Filter arch files from machine indpendent files.

IND_OBJS=$1
ARCH_OBJS=$2
ARCH_DIR=$3

RES=

for obj in $IND_OBJS ; do
    if [[ "$ARCH_OBJS" == *"$obj"* ]] ; then
        RES="$RES $ARCH_DIR/$obj"
    else
        RES="$RES $obj"
    fi
done

echo $RES
