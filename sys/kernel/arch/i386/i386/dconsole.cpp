/********************************************************************************************************************/

#include <string.h>
#include <kernel/arch/dconsole.h>

#include "cpu.h"

/********************************************************************************************************************/

#ifdef _USE_BOCHS
# define BOCHS_DEBUG_PORT 0xE9
#endif

#define COM_PORT 0x03F8

// Only with DLAB = 0
#define SERIAL_TRX       (COM_PORT + 0x00)

// Only with DLAB = 0
#define SERIAL_INT_CTL   (COM_PORT + 0x01)

// Only with DLAB = 1
#define SERIAL_DLA_LSB   (COM_PORT + 0x00)

// Only with DLAB = 1
#define SERIAL_DLA_MSB   (COM_PORT + 0x01)

// Readonly
#define SERIAL_INT_STAT  (COM_PORT + 0x02)

// Write only
#define SERIAL_FIFO_CTL  (COM_PORT + 0x02)
#define SERIAL_LINE_CTL  (COM_PORT + 0x03)
#define SERIAL_MOD_CTL   (COM_PORT + 0x04)
#define SERIAL_LINE_STAT (COM_PORT + 0x05)
#define SERIAL_MOD_STAT  (COM_PORT + 0x06)
#define SERIAL_SCRATCH   (COM_PORT + 0x07)

#define SERIAL_DLA_BIT 0x80

// Set 8-Bits, one stop bit, odd parity
#define SERIAL_CONTROL_FLAGS 0x0B

namespace DebugConsole
{
    int Init1(void)
    {
        // Disable interrupts, we're just going to manually poll.
        outb(SERIAL_INT_CTL, 0);

        // Enable and reset the FIFO
        outb(SERIAL_FIFO_CTL, 0x05);

        // Setup bit pattern, and enable writing ot the divisor latches
        outb(SERIAL_LINE_CTL, SERIAL_CONTROL_FLAGS | SERIAL_DLA_BIT);

        // Set up for 19200 buad
        outb(SERIAL_DLA_MSB, 0);
        outb(SERIAL_DLA_LSB, 6);

        // Now clear the DLA bit so we can read/write data
        outb(SERIAL_LINE_CTL, SERIAL_CONTROL_FLAGS);

        // Anounce ourselves
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
        return inb(BOCHS_DEBUG_PORT);
#endif

        // Remember this is a nonblocking method.
        if ((inb(SERIAL_LINE_STAT) & 0x01) == 0)
            return -1;

        return inb(SERIAL_TRX);
    }

    /************************************************************************************************************/

    int PutChar(char c)
    {
#ifdef _USE_BOCHS
        outb(BOCHS_DEBUG_PORT, c);
#endif

        while ((inb(SERIAL_LINE_STAT) & 0x20) == 0)
            ;

        outb(SERIAL_TRX, c);

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
