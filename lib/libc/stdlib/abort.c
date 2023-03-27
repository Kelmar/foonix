/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#ifdef __is_libk
//# include <kernel/flow.h>

// TODO: Put back into kernel header.
__attribute__((__noreturn__))
void panic(const char* message);
#endif

/*************************************************************************/

__attribute__((__noreturn__))
void abort(void)
{
#ifdef __is_libk
    // TODO: Add proper kernel panic
    panic("kernel: panic: abort()\n");
#else
    // TODO: Terminate the application like SIGABRT
    printf("abort()\n");
#endif

    for (;;) { }
    __builtin_unreachable();
}

/*************************************************************************/
