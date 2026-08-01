/*
 * Host stand-ins for kernel/flow.h's kpanic()/kassert(). In the real
 * kernel these route to the debug console and khalt(); here a violated
 * invariant reports what happened and aborts the test process instead.
 *
 * Deliberately doesn't include <sys/assert.h>: this file isn't built with
 * sys/include on its path (see CMakeLists.txt), since <stdlib.h> (for
 * abort()) needs to be the real host header, and the project's own
 * sys/include/sys/cdefs.h shares a name with -- and would shadow -- the
 * glibc sys/cdefs.h that <stdlib.h> relies on internally. The signatures
 * below are duplicated from sys/assert.h and must stay in sync with it.
 */
#include <stdlib.h>

#include "test_io.h"

__attribute__((noreturn)) void kpanic(const char *message);
__attribute__((noreturn)) void kassert(const char *test, const char *reason, int line, const char *file, const char *function);

void kpanic(const char *message)
{
    test_io_printf("KPANIC: %s\n", message);
    abort();
}

void kassert(const char *test, const char *reason, int line, const char *file, const char *function)
{
    test_io_printf(
        "KASSERT FAILED: %s\n  expression: %s\n  at %s:%d (%s)\n",
        reason, test, file, line, function
    );
    abort();
}
