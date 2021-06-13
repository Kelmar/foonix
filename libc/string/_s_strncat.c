/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/
/*
 * Just like strncat() except we take in the maximum possible size of the destination string as well as the source
 * string.  Plus we ALWAYS terminate with a NUL character.
 */
char* _s_strncat(char* dst, size_t dsz, const char* src, size_t ssz)
{
    char* end = memchr(dst, '\0', dsz);
    size_t dlen, slen;

    if (end == NULL)
    {
	/* Panic? */
	return NULL;
    }

    dlen = (size_t)(end - dst);
    slen = strnlen(src, ssz);

    /* Don't forget to account for NUL */
    if ((dlen + slen) > (dsz - 1))
	slen = dsz - (dlen + 1);

    memcpy(end, src, slen);
    end[slen] = '\0';

    return dst;
}

/********************************************************************************************************************/
