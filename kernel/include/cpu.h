/*************************************************************************/
/*
 * $Id: cpu.h 95 2010-10-18 03:54:14Z kfiresun $
 */
/*************************************************************************/

#ifndef __FOONIX_CPU_H__
#define __FOONIX_CPU_H__

/*************************************************************************/

#include "cdefs.h"
#include "stdint.h"
#include "cpudefs.h"
#include "multboot.h"

/*************************************************************************/

__BEGIN_DECLS

/*************************************************************************/

struct regs
{
    /* Segs are pushed last. */
    uint32_t gs, fs, es, ds;

    /* Pushed via pusha */
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;

    /* Pushed manually by our _isrX asm function. */
    uint32_t int_no, err_code;

    /* The CPU pushes these automatically */
    uint32_t eip, cs, eflags;
    uint32_t useresp, ss;
} __attribute__((packed));

typedef struct regs regs_t;

/*************************************************************************/

typedef void *io_port_t;
typedef void (*isr_handler_t)(struct regs *r);

/*************************************************************************/
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

/*************************************************************************/
/* Defined in machine.c */

/* Provides a spin count mechinizim around cli & sti. */
void block_interrupts(void);
void start_interrupts(void);

/* Functions for writing to the IO bus */
void bus_write_1(io_port_t base, int offset, uint8_t  value);
void bus_write_2(io_port_t base, int offset, uint16_t value);
void bus_write_4(io_port_t base, int offset, uint32_t value);

void bus_write_s1(io_port_t base, int offset, void *value, size_t bytes);
void bus_write_s2(io_port_t base, int offset, void *value, size_t bytes);
void bus_write_s4(io_port_t base, int offset, void *value, size_t bytes);

/* Functions for reading from the IO bus */
uint8_t  bus_read_1(io_port_t base, int offset);
uint16_t bus_read_2(io_port_t base, int offset);
uint32_t bus_read_4(io_port_t base, int offset);

void bus_read_s1(io_port_t base, int offset, void *value, size_t bytes);
void bus_read_s2(io_port_t base, int offset, void *value, size_t bytes);
void bus_read_s4(io_port_t base, int offset, void *value, size_t bytes);

/*************************************************************************/
/* Defined in desctab.c */

void init_descriptor_tables(void);
isr_handler_t set_isr_callback(uint8_t, isr_handler_t);

void dump_regs(const struct regs *r);

/*************************************************************************/
/* Defined in main.cpp */

/*
 * Halts the system with a "STOP ERROR" message.
 */
void kstop(const char *msg);

/*
 * Immediately halts the system with no messages what-so-ever.
 */
void khalt(void);

/*
 * Waits for the specified amount of time in miliseonds.
 */
void kwait(uint32_t msecs);

/*
 * Main entry point
 */
void kmain(multiboot_t *mbd, uintptr_t stackPtr);

/*************************************************************************/

/*
 * Checks for COND_ to be true, if false, then we call kstop with an ASSERT
 * failure and provide debugging information.
 */
#ifndef NDEBUG
# define ASSERT(COND_, REASON_)			    \
    do { if (!(COND_)) {			    \
	char buf[1024];				    \
	snkprintf(buf, sizeof(buf),		    \
	    "\n**** ASSERTION FAILURE ****\n"	    \
	    "    CONDITION: %s\n"		    \
	    "    FILE     : %s\n"		    \
	    "    LINE     : %d\n"		    \
	    "    REASON   : %s\n"		    \
	    "**** ASSERTION FAILURE ****\n",	    \
	    #COND_, __FILE__, __LINE__, REASON_);   \
	    kstop(buf); } } while (false)
#else
# define ASSERT(COND_, REASON_)
#endif

/*************************************************************************/

__END_DECLS

/*************************************************************************/

#endif /* __FOONIX_CPU_H__ */

/*************************************************************************/
