/********************************************************************************************************************/

#include <sys/assert.h>
#include <sys/cdefs.h>
#include <stddef.h>

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

ISABus::ISABus(int base, size_t size)
    : m_base(static_cast<uint16_t>((uint32_t)base))
    , m_size(size)
{
}

ISABus::~ISABus(void)
{
}

/********************************************************************************************************************/

uint8_t ISABus::byte(unsigned int offset)
{
    ASSERT(offset < m_size, "Bus write overflow");
    return x86::inb((uint16_t)(m_base + offset));
}

uint16_t ISABus::word(unsigned int offset)
{
    return x86::inw((uint16_t)m_base + offset);
}

uint32_t ISABus::dword(unsigned int offset)
{
    return x86::inl((uint16_t)m_base + offset);
}

/********************************************************************************************************************/

void ISABus::byte(unsigned int offset, uint8_t value)
{
    ASSERT(offset < m_size, "Bus write overflow");
    x86::outb(m_base + offset, value);
}

void ISABus::word(unsigned int offset, uint16_t value)
{
    x86::outw(m_base + offset, value);
}

void ISABus::dword(unsigned int offset, uint32_t value)
{
    x86::outl(m_base + offset, value);
}

/********************************************************************************************************************/
