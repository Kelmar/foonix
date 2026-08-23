/********************************************************************************************************************/
/*
 * Common instructions that do not vary from x32 and x64
 */
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_CPU_BASE_H__
#define __FOONIX_ARCH_X86_CPU_BASE_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

/********************************************************************************************************************/

namespace cpu
{
    /************************************************************************************************************/

    constexpr const size_t PageSize = PAGE_SIZE;

#undef PAGE_SIZE

    /************************************************************************************************************/

    static inline FORCE_INLINE
    void pause() { __asm__ volatile("pause"); }

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
        __asm__ ("inb %1,%0" : "=a" (data) : "d" (port));
        return data;
    }

    static inline FORCE_INLINE
    void outb(uint16_t port, uint8_t data)
    {
        __asm__ ("outb %0,%1" : : "a" (data), "d" (port));
    }

    static inline FORCE_INLINE
    uint16_t inw(uint16_t port)
    {
        uint16_t data;
        __asm__ ("inw %1,%0" : "=a" (data) : "d" (port));
        return data;
    }

    static inline FORCE_INLINE
    void outw(uint16_t port, uint16_t data)
    {
        __asm__ ("outw %0,%1" : : "a" (data), "d" (port));
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

    static inline FORCE_INLINE uintptr_t
    read_cr3(void)
    {
        uintptr_t rval;
        __asm__("movl %%cr3,%0" : "=r" (rval));
        return rval;
    }

    static inline FORCE_INLINE void
    load_cr3(uintptr_t value)
    {
        __asm__("movl %0,%%cr3" :: "r" (value));
    }

    static inline FORCE_INLINE void
    reload_cr3(void)
    {
        uint32_t value;
        __asm__("movl %%cr3,%0" : "=r" (value));
        __asm__("movl %0,%%cr3" :: "r" (value));
    }
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_CPU_BASE_H__ */

/********************************************************************************************************************/
