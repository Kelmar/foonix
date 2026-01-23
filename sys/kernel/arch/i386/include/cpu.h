/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_CPU_H__
#define __FOONIX_ARCH_I386_CPU_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stdint.h>

#include "cpudefs.h"

/********************************************************************************************************************/

#define MAX_32_ADDR 0x00000000FFFFFFFFull
#define PAGE_SIZE 4096

/********************************************************************************************************************/

typedef uint32_t register_t;

struct regs
{
    /* Segs are pushed last. */
    register_t gs, fs, es, ds;

    /* Pushed via pusha */
    register_t edi, esi, ebp, esp;
    register_t ebx, edx, ecx, eax;

    /* Pushed manually by our _isrX asm function. */
    register_t int_no, err_code;

    /* The CPU pushes these automatically */
    register_t eip, cs, eflags;
    register_t useresp, ss;
} __attribute__((packed));

typedef struct regs regs_t;

typedef void* io_port_t;
typedef void (*isr_handler_t)(struct regs* r);

/********************************************************************************************************************/

__BEGIN_EXTERN_C

/********************************************************************************************************************/
/* cpu.S functions */

/*
 * Reads the CPU counter.
 *
 * Parameters:
 * lo_value	- Pointer to a 32bit unsigned integer that receives the lower
 *		  word portion of the counter value.
 * hi_value	- Pointer to a 32bit unsigned integer that receives the higer
 *		  word portion of the counter value.
 *
 * Returns:
 * The lower 32bit unsigned value of the counter.
 *
 * Notes:
 * Both lo_value and hi_value maybe NULL, in which case the function's
 * return value can be used for the lo_value.
 */
uint32_t read_cpu_counter(uint32_t *lo_value, uint32_t *hi_value);

uint32_t read_eax(void);

static inline FORCE_INLINE uint32_t
read_cr0(void)
{
    uint32_t rval;
    __asm__("movl %%cr0,%0" : "=r" (rval));
    return rval;
}

static inline FORCE_INLINE uint32_t
read_cr2(void)
{
    uint32_t rval;
    __asm__("movl %%cr2,%0" : "=r" (rval));
    return rval;
}

static inline FORCE_INLINE uint32_t
read_cr3(void)
{
    uint32_t rval;
    __asm__("movl %%cr3,%0" : "=r" (rval));
    return rval;
}

static inline FORCE_INLINE uint32_t
read_cr4(void)
{
    uint32_t rval;
    __asm__("movl %%cr4,%0" : "=r" (rval));
    return rval;
}

static inline FORCE_INLINE void
load_cr0(uint32_t value)
{
    __asm__("movl %0,%%cr0" :: "r" (value));
}

static inline FORCE_INLINE void
load_cr3(uint32_t value)
{
    __asm__("movl %0,%%cr3" :: "r" (value));
}

static inline FORCE_INLINE void
load_cr4(uint32_t value)
{
    __asm__("movl %0,%%cr4" :: "r" (value));
}

static inline FORCE_INLINE void
reload_cr3(void)
{
    uint32_t value;
    __asm__("movl %%cr3,%0" : "=r" (value));
    __asm__("movl %0,%%cr3" :: "r" (value));
}

uint32_t read_ss(void);
uint32_t read_cs(void);

uint32_t read_ebp(void);
uint32_t read_esp(void);
uint32_t read_eip(void);

uint32_t read_eflags(void);

/*
 * Reads the RTC seconds value.
 *
 * This function is mostly used for timing.
 */
uint32_t read_rtc_second(void);

/*
 * Reads the current time of day from the RTC.
 *
 * The value returned is the number of seconds sense midnight.
 */
uint32_t read_rtc_time(void);

/*
 * A short software delay, the lenght of time this delays for is
 * highly dependent on the speed of the CPU.
 *
 * Intended for brief pauses when we need to wait for hardware
 * to respond.
 */
void soft_delay(void);

/**
 * Checks to see if the CPUID instruction is available on this CPU
 */
bool has_cpuid(void);

/********************************************************************************************************************/

__END_EXTERN_C

/********************************************************************************************************************/

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

static inline FORCE_INLINE void
stop_interrupts(void)
{
    __asm__("cli");
}

static inline FORCE_INLINE void
start_interrupts(void)
{
    __asm__("sti");
}

#if defined(NDEBUG) || !defined(_USE_BOCHS)
# define bochs_breakpoint()
#else
# define bochs_breakpoint() __asm__("xchgw %bx,%bx")
#endif 

/********************************************************************************************************************/

static inline void cpuid(uint32_t level, uint32_t& ax, uint32_t& bx, uint32_t& cx, uint32_t& dx)
{
    __asm__(
        "cpuid\n\t"
        : "=a"(ax), "=b"(bx), "=c"(cx), "=d"(dx)
        : "0"(level)
    );
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_CPU_H__ */

/********************************************************************************************************************/
