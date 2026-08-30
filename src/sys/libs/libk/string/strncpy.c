/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

char* PREFIX_(strncpy)(char* dst, const char* src, size_t size)
{
    size_t ssz = PREFIX_(strnlen)(src, size) + 1; // Include NUL

    ssz = ssz > size ? size : ssz;

    PREFIX_(memcpy)(dst, src, ssz);

    if (ssz < size)
    {
        // Fill remaining buffer with NUL per Linux man page.
        PREFIX_(memset)(dst + ssz, 0, size - ssz);
    }

    return dst;
}

/********************************************************************************************************************/
