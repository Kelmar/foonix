/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/
/*
 * Just like strncpy() except we take in the maximum possible size of the destination string as well as the source
 * string.  Plus we ALWAYS terminate with a NUL character.
 */
char* _s_strncpy(char* dst, size_t dsz, const char* src, size_t ssz)
{
    size_t mlen = (dsz > ssz) ? ssz : dsz;
    size_t slen;

    slen = strnlen(src, ssz) + 1; /* Don't forget to account for NUL */
    mlen = (mlen > slen) ? slen : mlen;

    memcpy(dst, src, mlen);


    // Pedantic safety, should be taken care of with copy of NUL from src to dst.
    // Could be an issue if the write is unexpected.
    dst[dsz - 1] = '\0'; 

    return dst;
}

/********************************************************************************************************************/
