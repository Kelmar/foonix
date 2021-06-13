/*************************************************************************/

#include <stdio.h>
#include <string.h>

#ifdef __is_libk
# include <kernel/tty.h>
#endif

/*************************************************************************/

int puts(const char *string)
{
#ifdef __is_libk
    terminal_writestr(string);
    terminal_putchar('\n');
#else
    // TODO: Implement standard C version
# error Need puts() implementation details.
#endif

    return strlen(string) + 1;
}

/*************************************************************************/