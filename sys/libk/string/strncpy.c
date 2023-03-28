/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

char* strncpy(char* dst, const char* src, size_t size)
{
    char* end = (char *)memchr(dst, '\0', size);
    size_t dlen, slen;

    if (end == NULL)
    {
	/* Panic? */
	return NULL;
    }

    dlen = (size_t)(end - dst);
    slen = strnlen(src, size);

    /* Don't forget to account for NUL */
    if ((dlen + slen) > (size - 1))
	slen = size - (dlen + 1);

    memcpy(end, src, slen);
    end[slen] = '\0';

    return dst;
}

/********************************************************************************************************************/
