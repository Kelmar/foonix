#!/bin/sh

CWD=`pwd`

if [ ! -d tools/bin ] ; then
  mkdir -p tools/bin
fi

PATH=$CWD/tools/bin:$PATH
export PATH

#echo "PATH: $PATH"

