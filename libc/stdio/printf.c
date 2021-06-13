/********************************************************************************************************************/

#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/********************************************************************************************************************/

int printf(const char* restrict format, ...)
{
    va_list args;
    int rval;

    va_start(args, format);
    rval = vprintf(format, args);
    va_end(args);

    return rval;
}

/********************************************************************************************************************/
