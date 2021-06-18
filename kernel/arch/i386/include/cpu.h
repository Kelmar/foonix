/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_CPU_H__
#define __FOONIX_ARCH_I386_CPU_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stdint.h>

#include "cpudefs.h"

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

uint32_t read_cr0(void);
uint32_t read_cr2(void);
uint32_t read_cr3(void);
uint32_t read_cr4(void);

void load_cr0(uint32_t value);
void load_cr3(uint32_t value);
void load_cr4(uint32_t value);

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

static inline void stop_interrupts(void)
{
    __asm __volatile("cli");
}

static inline void start_interrupts(void)
{
    __asm __volatile("sti");
}

/********************************************************************************************************************/

static inline void cpuid(uint32_t level, uint32_t& ax, uint32_t& bx, uint32_t& cx, uint32_t& dx)
{
    __asm __volatile(
        "cpuid\n\t"
        : "=a"(ax), "=b"(bx), "=c"(cx), "=d"(dx)
        : "0"(level)
    );
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_CPU_H__ */

/********************************************************************************************************************/
