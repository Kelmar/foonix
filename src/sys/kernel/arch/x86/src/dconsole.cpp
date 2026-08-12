/********************************************************************************************************************/

#include <string.h>
#include <kernel/arch/dconsole.h>

#include "uart.h"
#include "cpu.h"

/********************************************************************************************************************/

#ifdef _USE_BOCHS
# define BOCHS_DEBUG_PORT 0xE9
#endif

namespace DebugConsole
{
    int Init1(void)
    {
        // Setup debugging serial port.
        // We assume that the serial port is compatible with a 16550.

#ifndef _USE_BOCHS
        uart::init();
#endif

        // Announce ourselves
        const char msg[] = "FooNIX Ready!\r\n";
        PutString(msg, strnlen(msg, sizeof(msg)));

        return 0;
    }

    /************************************************************************************************************/

    int Init2(void)
    {
        return 0;
    }

    /************************************************************************************************************/

    int ReadChar(void)
    {
#ifdef _USE_BOCHS
        return cpu::inb(BOCHS_DEBUG_PORT);
#else
        return uart::read_char();
#endif
    }

    /************************************************************************************************************/

    int PutChar(char c)
    {
#ifdef _USE_BOCHS
        cpu::outb(BOCHS_DEBUG_PORT, c);
#else
        uart::write_char(c);
#endif

        return 0;
    }

    /************************************************************************************************************/

    void Write(const char *str, size_t len)
    {
        const char *s = str;
        
        for (size_t i = 0; i < len; ++i, ++s)
            PutChar(*s);
    }

    /************************************************************************************************************/

    void PutString(const char *str, size_t len)
    {
        const char *s = str;

        for (size_t i = 0; i < len && *s != '\0'; ++i, ++s)
            PutChar(*s);
    }

    /************************************************************************************************************/

    void DumpUInt64Hex(uint64_t i)
    {
        for (int index = 0; index < 16; ++index)
        {
            if (index > 0 && (index % 4) == 0)
                PutChar('_');

            int off = (15 - index) * 4;
            int p = (i >> off) & 0x0F;
            PutChar("0123456789ABCDEF"[p]);
        }
    }
}

/********************************************************************************************************************/
