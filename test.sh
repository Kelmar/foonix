#!/bin/sh

set -e

cmake -S src/test2 -B build/test2
cmake --build build/test2
ctest --test-dir build/test2 --output-on-failure

