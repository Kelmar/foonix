#!/bin/sh

set -e

cmake -S src/tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure

