/********************************************************************************************************************/

#include "uart.h"
#include "cpu.h"

/********************************************************************************************************************/

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

/********************************************************************************************************************/

Kernel::ErrorCode uart::init()
{
    // Setup debugging serial port.
    // We assume that the serial port is compatible with a 16550.

    // Disable interrupts, we're just going to manually poll.
    cpu::outb(SERIAL_INT_CTL, 0);

    // Enable and reset the FIFO
    cpu::outb(SERIAL_FIFO_CTL, 0x05);

    // Setup bit pattern, and enable writing ot the divisor latches
    cpu::outb(SERIAL_LINE_CTL, SERIAL_CONTROL_FLAGS | SERIAL_DLA_BIT);

    // Set up for 19200 baud
    cpu::outb(SERIAL_DLA_MSB, 0);
    cpu::outb(SERIAL_DLA_LSB, 6);

    // Now clear the DLA bit so we can read/write data
    cpu::outb(SERIAL_LINE_CTL, SERIAL_CONTROL_FLAGS);

    return Kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/

int uart::read_char()
{
    // Remember this is a nonblocking method.
    if ((cpu::inb(SERIAL_LINE_STAT) & 0x01) == 0)
        return -1;

    return cpu::inb(SERIAL_TRX);
}

/********************************************************************************************************************/

void uart::write_char(char c)
{
    while ((cpu::inb(SERIAL_LINE_STAT) & 0x20) == 0)
        ;

    cpu::outb(SERIAL_TRX, c);
}

/********************************************************************************************************************/
