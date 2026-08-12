/********************************************************************************************************************/
/*
 * Common instructions that do not vary from x32 and x64
 */
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_CPU_BASE_H__
#define __FOONIX_ARCH_X86_CPU_BASE_H__

/********************************************************************************************************************/

namespace cpu
{
    static inline FORCE_INLINE
    void pause() { __asm __volatile("pause"); }

    static inline FORCE_INLINE
    void stop_interrupts(void)
    {
        __asm__("cli");
    }

    static inline FORCE_INLINE
    void start_interrupts(void)
    {
        __asm__("sti");
    }

    static inline FORCE_INLINE
    uint8_t inb(uint16_t port)
    {
        uint8_t data;
        __asm ("inb %1,%0" : "=a" (data) : "d" (port));
        return data;
    }

    static inline FORCE_INLINE
    void outb(uint16_t port, uint8_t data)
    {
        __asm ("outb %0,%1" : : "a" (data), "d" (port));
    }

    static inline FORCE_INLINE
    uint16_t inw(uint16_t port)
    {
        uint16_t data;
        __asm ("inw %1,%0" : "=a" (data) : "d" (port));
        return data;
    }

    static inline FORCE_INLINE
    void outw(uint16_t port, uint16_t data)
    {
        __asm ("outw %0,%1" : : "a" (data), "d" (port));
    }

    static inline FORCE_INLINE
    uint32_t inl(uint16_t port)
    {
        uint32_t data;
        __asm__("inl %1,%0" : "=a" (data) : "d" (port));
        return data;
    }

    static inline FORCE_INLINE
    void outl(uint16_t port, uint32_t data)
    {
        __asm__("outl %0,%1" : : "a" (data), "d" (port));
    }
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_CPU_BASE_H__ */

/********************************************************************************************************************/
