/********************************************************************************************************************/

#include <sys/assert.h>
#include <sys/cdefs.h>
#include <stddef.h>

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

bus::bus(io_port_t base, size_t size)
    : m_base(static_cast<uint16_t>((uint32_t)base))
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
    return cpu::inb((uint16_t)(m_base + offset));
}

uint16_t bus::word(unsigned int offset)
{
    return cpu::inw((uint16_t)m_base + offset);
}

uint32_t bus::dword(unsigned int offset)
{
    return cpu::inl((uint16_t)m_base + offset);
}

/********************************************************************************************************************/

void bus::byte(unsigned int offset, uint8_t value)
{
    ASSERT(offset < m_size, "Bus write overflow");
    cpu::outb(m_base + offset, value);
}

void bus::word(unsigned int offset, uint16_t value)
{
    cpu::outw(m_base + offset, value);
}

void bus::dword(unsigned int offset, uint32_t value)
{
    cpu::outl(m_base + offset, value);
}

/********************************************************************************************************************/
