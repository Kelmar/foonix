#include <stddef.h>
#include <stdlib.h>

void* malloc(size_t sz)
{
    (void)sz;

#ifdef __is_libk
#else
#endif

    return NULL;
}