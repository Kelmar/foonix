#!/bin/sh

set -e

cmake -S tests -B tests/build 
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
