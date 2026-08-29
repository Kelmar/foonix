/********************************************************************************************************************/

#include <string.h>

/********************************************************************************************************************/

void* memmove(void* dest, const void* source, size_t size)
{
    unsigned char* dst = dest;
    const unsigned char* src = source;

    if (size == 0)
        return dest; // Nothing to copy

    if (dst < src)
    {
        for (size_t i = 0; i < size; ++i)
            dst[i] = src[i];
    }
    else
    {
        for (size_t i = size - 1; i < size; --i)
            dst[i] = src[i];
    }

    return dest;
}

/********************************************************************************************************************/
