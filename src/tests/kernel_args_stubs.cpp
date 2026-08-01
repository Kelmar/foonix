/*
 * Host stand-ins for what kernel_args.cpp normally leans on from the rest
 * of the kernel: kpanic() (called by KnockoutUsedMemory() when it runs out
 * of map entries) and Debug::PrintF() (called only by ShowAvailableMemory(),
 * which these tests don't exercise -- a no-op is enough to link).
 *
 * Deliberately doesn't include any project header (matches tests/kstubs.c's
 * reasoning): this file needs the real host <cstdlib> for abort(), and the
 * project's own sys/include/sys/cdefs.h would shadow glibc's own if
 * sys/include were on this file's search path. kpanic()'s signature below
 * must stay in sync with kernel/flow.h's declaration.
 */
#include <cstdlib>

#include "test_io.h"

extern "C" [[noreturn]] void kpanic(const char *message)
{
    test_io_printf("KPANIC: %s\n", message);
    abort();
}

namespace Debug
{
    void PrintF(const char *, ...)
    {
    }
}
