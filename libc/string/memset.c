/*************************************************************************/

#include <string.h>

/*************************************************************************/

void* memset(void* buf, int val, size_t size)
{
    unsigned char* b = buf = (unsigned char *)buf;

    for (size_t i = 0; i < size; ++i)
        b[i] = (unsigned char)val;

    return buf;
}

/*************************************************************************/
