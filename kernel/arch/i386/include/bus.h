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

    uint8_t read1(int offset);
    uint16_t read2(int offset);
    uint32_t read4(int offset);

    void write1(int offset, uint8_t value);
    void write2(int offset, uint16_t value);
    void write4(int offset, uint32_t value);
};

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_BUS_H__ */

/********************************************************************************************************************/