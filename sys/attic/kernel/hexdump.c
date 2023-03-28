/*************************************************************************/
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "kernio.h"
#include "kheap.h"

/*************************************************************************/

void HexDump(void *block, size_t sz)
{
    char obuf[17];
    size_t chunk;
    size_t i, j;
    char *data;
    int c;

    data = (char *)block;

    kprintf("Hex dump of %p - %d bytes\n", block, sz);

    putstr(" Values                  -                         : Characters\n");
    putstr("======================================================================\n");

    for (i = 0; i < sz; i += chunk)
    {
        if ((i + 16) > sz)
            chunk = sz - i;
        else
            chunk = 16;

        putstr(" ");
        for (j = 0; j < chunk; ++j)
        {
            c = data[i + j] & 0xFF;
            obuf[j] = (c >= 32) ? (char)c : '.';

            kprintf("%02X ", c);

            if (j == 7)
                putstr("- ");
        }

        for (; j < 16; ++j)
        {
            putstr("   ");
            obuf[j] = ' ';
        }

        obuf[16] = '\0';
        kprintf(": %s\n", obuf);
    }
}

/*************************************************************************/
