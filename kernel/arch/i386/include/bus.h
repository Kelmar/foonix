/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_BUS_H__
#define __FOONIX_ARCH_I386_BUS_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"

class bus
{
private:
    uint8_t* m_base;
    size_t m_size;

public:
    /* constructor */ bus(io_port_t base, size_t size);
    virtual ~bus(void);

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