/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_64_CPU_H__
#define __FOONIX_ARCH_X86_64_CPU_H__

/********************************************************************************************************************/

#define MAX_ADDR 0x00000000FFFFFFFFull

static inline FORCE_INLINE uint8_t
inb(uint16_t port)
{
    uint8_t data;
    __asm ("inb %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline FORCE_INLINE void
outb(uint16_t port, uint8_t data)
{
    __asm ("outb %0,%1" : : "a" (data), "d" (port));
}

static inline FORCE_INLINE uint16_t
inw(uint16_t port)
{
    uint16_t data;
    __asm ("inw %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline FORCE_INLINE void
outw(uint16_t port, uint16_t data)
{
    __asm ("outw %0,%1" : : "a" (data), "d" (port));
}

static inline FORCE_INLINE uint32_t
inl(uint16_t port)
{
    uint32_t data;
    __asm__("inl %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline FORCE_INLINE void
outl(uint16_t port, uint32_t data)
{
    __asm__("outl %0,%1" : : "a" (data), "d" (port));
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_64_CPU_H__ */

/********************************************************************************************************************/
