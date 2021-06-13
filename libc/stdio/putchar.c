/*************************************************************************/

#include <string.h>

#ifdef __is_libk
# include <kernel/tty.h>
#endif

/*************************************************************************/

int putchar(int c)
{
#ifdef __is_libk
    terminal_putchar(c);
#else
    // TODO: Implement lib C version.
#endif

    return c;
}

/*************************************************************************/
