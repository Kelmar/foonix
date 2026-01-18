/*************************************************************************/
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "kernio.h"
#include "kheap.h"

/*************************************************************************/
/* Defined in cpu.S */

/* Disables interrupts. */
void cli(void);

/* Enables interrupts. */
void sti(void);

/* Writes a byte, word, or lword to a hardware port. */
void outb(io_port_t port, uint8_t  byte);
void outw(io_port_t port, uint16_t word);
void outl(io_port_t port, uint32_t dword);

/* Reads a byte, word, or lword from a hardware port. */
uint8_t  inb(io_port_t port);
uint16_t inw(io_port_t port);
uint32_t inl(io_port_t port);

/* Writes a buffer out to a hardware port. */
void outsb(io_port_t port, void *data, size_t bytes);
void outsw(io_port_t port, void *data, size_t bytes);
void outsl(io_port_t port, void *data, size_t bytes);

/* Reads a buffer in from a hardware port. */
void insb(io_port_t port, void *data, size_t bytes);
void insw(io_port_t port, void *data, size_t bytes);
void insl(io_port_t port, void *data, size_t bytes);

/*************************************************************************/
/*
 * We start with interrupts initially disabled, the main function will
 * spin this down and restart interrupts when the time is right.
 */
static uint32_t s_spin_count = 1;

/*************************************************************************/

void block_interrupts(void)
{
    if (s_spin_count == 0)
	cli();

    if (s_spin_count == 0xFFFFFFFF)
	kstop("Interrupt spin count reached max.");

    ++s_spin_count;
}

/*************************************************************************/

void start_interrupts(void)
{
    if (s_spin_count > 0)
    {
	if (--s_spin_count == 0)
	    sti();
    }
    else
	kprintf("WARNING: inerrupts were unblocked with spin count of 0.\n");
}

/*************************************************************************/

void bus_write_1(io_port_t port, int offset, uint8_t value)
{
    outb(port + offset, value);
}

void bus_write_2(io_port_t port, int offset, uint16_t value)
{
    outw(port + offset, value);
}

void bus_write_4(io_port_t port, int offset, uint32_t value)
{
    outl(port + offset, value);
}

/*************************************************************************/

void bus_write_s1(io_port_t port, int offset, void *value, size_t bytes)
{
    outsb(port + offset, value, bytes);
}

void bus_write_s2(io_port_t port, int offset, void *value, size_t bytes)
{
    outsw(port + offset, value, bytes);
}

void bus_write_s4(io_port_t port, int offset, void *value, size_t bytes)
{
    outsl(port + offset, value, bytes);
}

/*************************************************************************/

uint8_t bus_read_1(io_port_t port, int offset)
{
    return inb(port + offset);
}

uint16_t bus_read_2(io_port_t port, int offset)
{
    return inw(port + offset);
}

uint32_t bus_read_4(io_port_t port, int offset)
{
    return inl(port + offset);
}

/*************************************************************************/

void bus_read_s1(io_port_t port, int offset, void *value, size_t bytes)
{
    insb(port + offset, value, bytes);
}

void bus_read_s2(io_port_t port, int offset, void *value, size_t bytes)
{
    insw(port + offset, value, bytes);
}

void bus_read_s4(io_port_t port, int offset, void *value, size_t bytes)
{
    insl(port + offset, value, bytes);
}

/*************************************************************************/
