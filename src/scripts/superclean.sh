#!/bin/sh

# Go through and really scrub the system clean

find . -type f -name cmake_install.cmake -exec rm -fv {} \;
find . -type d -name CMakeFiles -exec rm -rvf {} \;
find . -type f -name CMakeCache.txt -exec rm -fv {} \;
find . -type f -name build.ninja -exec rm -fv {} \;
#find . -type f -name Makefile -exec rm -fv {} \;

#rm -rf tools

rm -rf sysroot
