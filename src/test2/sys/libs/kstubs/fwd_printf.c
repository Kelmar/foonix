
#include <stdarg.h>
#include <stdio.h>

void fwd_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
