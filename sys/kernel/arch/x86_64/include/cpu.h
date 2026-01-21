/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_64_CPU_H__
#define __FOONIX_ARCH_X86_64_CPU_H__

/********************************************************************************************************************/

#define MAX_ADDR 0x00000000FFFFFFFFull

static inline uint8_t
inb(uint16_t port)
{
    (void)port;
    return 0;
}

static inline void
outb(uint16_t port, uint8_t data)
{
    (void)port;
    (void)data;
}

static inline uint16_t
inw(uint16_t port)
{
    (void)port;
    return 0;
}

static inline void
outw(uint16_t port, uint16_t data)
{
    (void)port;
    (void)data;
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_64_CPU_H__ */

/********************************************************************************************************************/
