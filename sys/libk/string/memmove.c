/*************************************************************************/

#include <string.h>

/*************************************************************************/

void* memmove(void* dest, const void* source, size_t size)
{
    unsigned char* dst = dest;
    const unsigned char* src = source;

    if (dst < src)
    {
        for (size_t i = 0; i < size; ++i)
            dst[i] = src[i];
    }
    else
    {
        for (size_t i = size; i != 0; --i)
            dst[i] = src[i];
    }

    return dest;
}

/*************************************************************************/
