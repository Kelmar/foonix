/********************************************************************************************************************/

#include <sys/assert.h>
#include <sys/cdefs.h>
#include <stddef.h>

#include "bus.h"

/********************************************************************************************************************/
/* Defined in cpu.S */

__BEGIN_EXTERN_C

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
void outsb(io_port_t port, void* data, size_t bytes);
void outsw(io_port_t port, void* data, size_t bytes);
void outsl(io_port_t port, void* data, size_t bytes);

/* Reads a buffer in from a hardware port. */
void insb(io_port_t port, void* data, size_t bytes);
void insw(io_port_t port, void* data, size_t bytes);
void insl(io_port_t port, void* data, size_t bytes);

__END_EXTERN_C

/********************************************************************************************************************/

bus::bus(io_port_t base, size_t size)
    : m_base(static_cast<uint8_t*>(base))
    , m_size(size)
{
}

bus::~bus(void)
{
}

/********************************************************************************************************************/

uint8_t bus::byte(unsigned int offset)
{
    ASSERT(offset < m_size, "Bus write overflow");
    return inb(m_base + offset);
}

uint16_t bus::word(unsigned int offset)
{
    return inw(m_base + offset);
}

uint32_t bus::dword(unsigned int offset)
{
    return inl(m_base + offset);
}

/********************************************************************************************************************/

void bus::byte(unsigned int offset, uint8_t value)
{
    ASSERT(offset < m_size, "Bus write overflow");
    outb(m_base + offset, value);
}

void bus::word(unsigned int offset, uint16_t value)
{
    outw(m_base + offset, value);
}

void bus::dword(unsigned int offset, uint32_t value)
{
    outl(m_base + offset, value);
}

/********************************************************************************************************************/
