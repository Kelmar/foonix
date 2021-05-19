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

	obuf[0] = '\0';
	putstr(" ");
	for (j = 0; j < chunk; ++j)
	{
	    c = data[i + j] & 0xFF;

	    if (c >= 32)
		obuf[j] = (char)c;
	    else
		obuf[j] = '.';

	    kprintf("%02X ", c);

	    if (j == 7)
		putstr("- ");
	}

	for (;j < 16; ++j)
	    putstr("   ");

	obuf[16] = '\0';
	kprintf(": %s\n", obuf);
    }
}

/*************************************************************************/
