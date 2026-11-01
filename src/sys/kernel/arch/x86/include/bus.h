/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_BUS_H__
#define __FOONIX_ARCH_I386_BUS_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"

class ISABus
{
private:
    uint16_t m_base;
    size_t m_size;

public:
    /* constructor */ ISABus(int base, size_t size);
    virtual ~ISABus(void);

    uint8_t byte(unsigned int offset);
    uint16_t word(unsigned int offset);
    uint32_t dword(unsigned int offset);

    void byte(unsigned int offset, uint8_t value);
    void word(unsigned int offset, uint16_t value);
    void dword(unsigned int offset, uint32_t value);
};

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_BUS_H__ */

/********************************************************************************************************************/