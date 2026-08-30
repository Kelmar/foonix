/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

char* PREFIX_(strncat)(char* buf, const char* src, size_t size)
{
    size_t blen = strnlen(buf, size);
    size_t ssz = strnlen(src, size) + 1; // Be sure to include the NUL character.

    if (blen >= size)
        return buf;

    size_t t = (blen + ssz) - size;

    if (t > 0 && t < size)
    {
        // Not enough space, trim to correct length.
        ssz -= t;
    }

    memcpy(buf + blen, src, ssz);

    // Pedantic safety, could be an issue if the write is unexpected.
    buf[size - 1] = '\0';

    return buf;
}

/********************************************************************************************************************/
