/*
 * Deliberately built without sys/include on its path (see CMakeLists.txt),
 * so these are the REAL host headers, not the project's freestanding
 * subset -- this is the one place in the test binary allowed to assume a
 * hosted C library.
 */
#include <stdarg.h>
#include <stdio.h>

#include "test_io.h"

void test_io_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}
