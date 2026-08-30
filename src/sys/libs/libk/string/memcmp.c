/********************************************************************************************************************/

#include <string.h>

/********************************************************************************************************************/

int PREFIX_(memcmp)(const void* aptr, const void* bptr, size_t size)
{
    const unsigned char *a = (const unsigned char *)aptr;
    const unsigned char *b = (const unsigned char *)bptr;

    for (size_t i = 0; i < size; ++i)
    {
        int diff = (int)a[i] - (int)b[i];

        if (diff != 0)
            return diff;
    }

    return 0;
}

/********************************************************************************************************************/
