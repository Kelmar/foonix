/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

char* strncpy(char* dst, const char* src, size_t size)
{
    size_t ssz = strnlen(src, size) + 1; // Include NUL

    ssz = ssz > size ? size : ssz;

    memcpy(dst, src, ssz);

    if (ssz < size)
    {
        // Fill remaining buffer with NUL per Linux man page.
        memset(dst + ssz, 0, size - ssz);
    }

    return dst;
}

/********************************************************************************************************************/
