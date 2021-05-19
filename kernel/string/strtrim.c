/*************************************************************************/
/*************************************************************************/

#include "cdefs.h"
#include "stdlib.h"
#include "string.h"

/*************************************************************************/
/*
 * Trims a string of leading and trailing whitespace.
 */
char *strtrim(char *str, size_t sz)
{
    size_t len = strnlen(str, sz);
    char *p = str;
    size_t i;

    /* First strip leading whitespace. */
    i = 0;
    while (((*p == ' ') || (*p == '\t')) && (i < len))
    {
	++p;
        ++i;
    }

    if (i == len)
    {
       /* We reached the maximum length of the string, terminate and exit. */
       *str = '\0';
       return str;
    }

    if (p != str)
    {
	len -= (p - str);
	memcpy(str, p, len);
    }

    /* Now strip trailing whitespace. */

    p = str + len - 1;
    while ((p > str) && ((*p == ' ') || (*p == '\t')))
	--p;

    p[1] = '\0';

    return str;
}

/*************************************************************************/
