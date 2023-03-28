/********************************************************************************************************************/

#include <stdio.h>

#ifdef __is_libk
//# include <kernel/tty.h>
#endif

/********************************************************************************************************************/

int vprintf(const char* restrict fmt, va_list args)
{
    char buf[1024];
    int rval = 0;

    rval = vsnprintf(buf, sizeof(buf), fmt, args);

#ifdef __is_libk
    //terminal_writestr(buf);
#else
    // TODO: Implement standard C version
# error Need vprintf() implementation details.
#endif

    return rval;
}

/********************************************************************************************************************/
