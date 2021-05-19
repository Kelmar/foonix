/*************************************************************************/
/*
 * $Id: inter.c 64 2010-09-03 03:34:41Z kfiresun $
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "kernio.h"
#include "kheap.h"

/*************************************************************************/
/* Defined in cpu.S */

/* Disables interrupts. */
void cli(void);

/* Enables interrupts. */
void sti(void);

/*************************************************************************/
/*
 * We start with interrupts initially disabled, the main function will
 * spin this down and restart interrupts when the time is right.
 */
static uint32_t s_spin_count = 1;

/*************************************************************************/

void block_interrupts(void)
{
    if (s_spin_count == 0)
	cli();

    if (s_spin_count == 0xFFFFFFFF)
	kstop("Interrupt spin count reached max.");

    ++s_spin_count;
}

/*************************************************************************/

void start_interrupts(void)
{
    if (s_spin_count > 0)
    {
	if (--s_spin_count == 0)
	    sti();
    }
    else
	kprintf("WARNING: inerrupts were unblocked with spin count of 0.\n");
}

/*************************************************************************/
