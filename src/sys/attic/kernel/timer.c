/*************************************************************************/
/*
 * $Id: timer.c 44 2010-02-19 23:53:25Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"

#include "stdint.h"
#include "string.h"
#include "stdlib.h"

#include "kernio.h"
#include "ktime.h"

#include "process.h"

/*************************************************************************/

#define TIMER_PORTBASE	0x0040
#define TIMER_COUNTER0	((io_port_t)TIMER_PORTBASE + 0)
#define TIMER_COUNTER1	((io_port_t)TIMER_PORTBASE + 1)
#define TIMER_COUNTER2	((io_port_t)TIMER_PORTBASE + 2)
#define TIMER_MODE	((io_port_t)TIMER_PORTBASE + 3)

#define TIMER_SEL0	0x00	/* Select counter 0 */
#define TIMER_SEL1	0x40	/* Select counter 1 */
#define TIMER_SEL2	0x80	/* Select counter 2 */
#define TIMER_INTTC	0x00	/* mode 0, int on terminal cnt */
#define TIMER_ONESHOT	0x02	/* mode 1, one shot */
#define TIMER_RATEGEN	0x04	/* mode 2, rate generator */
#define TIMER_SQWAVE	0x06	/* mode 3, square wave */
#define TIMER_SWSTROBE	0x08	/* mode 4, s/w triggered strobe */
#define TIMER_HWSTROBE	0x0A	/* mode 5, h/w triggered strobe */
#define TIMER_LATCH	0x00	/* Latch counter for reading */
#define TIMER_LSB	0x10	/* r/w counter LSB */
#define TIMER_MSB	0x20	/* r/w counter MSB */
#define TIMER_16BIT	0x30	/* r/w counter 16 bits, LSB first */
#define TIMER_BCD	0x01	/* Count in BCD */

/*************************************************************************/

/* Change this if needed. */
#define TIMER_FREQ	100 /* in Hz */

/* Physicial timer's frequency. */
#define TIMER_PHYSFREQ  1193180 /* don't change this! */

#define HZ (TIMER_PHYSFREQ / 10000)

/*************************************************************************/
/*
 * Keeps track of how many ticks that the system has been running for.
 */
static uint32_t s_timer_ticks = 0;

static uint32_t s_freq = 18;
static uint32_t s_freq_cnt = 0;

static uint8_t  s_secs  = 0;
static uint8_t  s_mins  = 0;
static uint32_t s_hours = 0;

static int s_debsect = -1;

/*************************************************************************/

static void timer_handler(struct regs *r)
{
    UNUSED(r);

    ++s_timer_ticks;

    if ((s_timer_ticks % s_freq) == 0)
    {
	char buf[16];
	int i;

	++s_secs;
	while (s_secs >= 60)
	{
	    ++s_mins;
	    s_secs -= 60;
	}

	while (s_mins >= 60)
	{
	    ++s_hours;
	    s_mins -= 60;
	}

	i = snkprintf(
	    buf, sizeof(buf),
	    "%d:%02d:%02d",
	    s_hours, s_mins, s_secs);

	set_debug_section_info(s_debsect, buf, i);
    }

    run_scheduler(r);
}

/*************************************************************************/
/*
 * Set's the PIT's timer frequency in HZ.
 */
static void set_timer_freq(int freq)
{
    int divis;

    /*
     * This unusual operation provides us with an extra bit we can use to
     * round the integer division up or down.  Correctly rounding this will
     * grant us a bit more accuracy with our timer's book keeping.
     */
    divis = (TIMER_PHYSFREQ << 1) / freq;
    divis = (divis >> 1) + (divis & 1);

    block_interrupts();

    /* Adjust the clock rate */
    bus_write_1(TIMER_MODE, 0, TIMER_SEL0 | TIMER_16BIT | TIMER_RATEGEN);
    bus_write_1(TIMER_COUNTER0, 0, divis & 0xFF);
    bus_write_1(TIMER_COUNTER0, 0, (divis >> 8) & 0xFF);

    s_freq = freq;
    s_freq_cnt = divis ? divis : 0xFFFF;

    start_interrupts();
}

/*************************************************************************/
/*
 * Installs the timer interrupt handler.
 */
void init_timer(void)
{
    s_debsect = get_debug_section();

    set_timer_freq(TIMER_FREQ);

    /* Install handler */
    set_isr_callback(IRQISR(0), timer_handler);

    kprintf("Timer initialized\n");
}

/*************************************************************************/

static int get_rtc_ticks(void)
{
    uint8_t lo, hi;

    block_interrupts();
    bus_write_1(TIMER_MODE, 0, TIMER_SEL0 | TIMER_LATCH);
    lo = bus_read_1(TIMER_COUNTER0, 0);
    hi = bus_read_1(TIMER_COUNTER0, 0);
    start_interrupts();

    return (hi << 8) | lo;
}

/*************************************************************************/
/*
 * Waits for a specified period of time in miliseconds.
 */
void kwait(uint32_t msecs)
{
    uint32_t counter = 0;
    int old, ticks;
    int delta;

    /* Include the math below as part of the delay. */
    old = get_rtc_ticks();

    /*
     * Adjust msecs to account for number of counter adjustments that
     * are expected to happen in the given amount of time.
     */
    delta = msecs / 1000;		/* Seconds */
    ticks = (msecs % 1000) * 100;	/* uSeconds */

    msecs = delta * TIMER_PHYSFREQ +
	ticks * (TIMER_PHYSFREQ / 1000000) +
	ticks * ((TIMER_PHYSFREQ % 1000000) / 1000) / 1000 +
	ticks * (TIMER_PHYSFREQ % 1000) / 1000000;

    while (counter < msecs)
    {
	asm volatile("pause"::); /* Keep the CPU from eating itself */

	ticks = get_rtc_ticks();

	if (ticks > old)
	    delta = s_freq_cnt - (ticks - old);
	else
	    delta = old - ticks;

	old = ticks;

        counter += delta;
    }
}

/*************************************************************************/

void init_ktimer(ktimer_t *timer, uint32_t msecs)
{
    timer->msecs = msecs;
    reset_ktimer(timer);
}

/*************************************************************************/

bool_t check_ktimer(ktimer_t *timer)
{
    uint32_t ticks, delta;

    asm volatile("pause"::); /* Keep the CPU from eating itself. */

    if (timer->counter >= timer->target)
	return false; /* Timer has already expired. */

    ticks = get_rtc_ticks();

    if (ticks > timer->last_read)
	delta = s_freq_cnt - (ticks - timer->last_read);
    else
	delta = timer->last_read - ticks;

    timer->last_read = ticks;

    timer->counter += delta;

    return (timer->counter < timer->target) ? true : false;
}

/*************************************************************************/

void reset_ktimer(ktimer_t *timer)
{
    int secs, usecs;

    /* Include the math below as part of the delay. */
    timer->last_read = get_rtc_ticks();
    timer->counter = 0;

    /*
     * Adjust msecs to account for number of counter adjustments that
     * are expected to happen in the given amount of time.
     */
    secs  = timer->msecs / 1000;
    usecs = (timer->msecs % 1000) * 100;

    timer->target = secs * TIMER_PHYSFREQ +
	usecs * (TIMER_PHYSFREQ / 1000000) +
	usecs * ((TIMER_PHYSFREQ % 1000000) / 1000) / 1000 +
	usecs * (TIMER_PHYSFREQ % 1000) / 1000000;
}

/*************************************************************************/
