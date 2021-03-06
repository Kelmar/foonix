/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

char* strncat(char* buf, const char* src, size_t size)
{
    size_t ssz = strnlen(src, size);

    ssz = ssz < size ? ssz : size;

    memcpy(buf, src, ssz);
    return buf;
}

/********************************************************************************************************************/
