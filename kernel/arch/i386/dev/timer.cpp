/********************************************************************************************************************/

#include <stdio.h>

#include "bus.h"
#include "cpu.h"
#include "idt.h"

/********************************************************************************************************************/

namespace
{
    const io_port_t TIMER_PORTBASE = (io_port_t)0x0040;

    const int TIMER_COUNTER0 = 0;
    const int TIMER_COUNTER1 = 1;
    const int TIMER_COUNTER2 = 2;
    const int TIMER_MODE     = 3;

    /************************************************************************************************************/

    enum timer_command
    {
        TIMER_SEL0      = 0x00, // Select counter 0
        TIMER_SEL1      = 0x40, // Select counter 1
        TIMER_SEL2      = 0x80, // Select counter 2
        TIMER_INTTC     = 0x00, // mode 0, int on terminal cnt
        TIMER_ONESHOT   = 0x02, // mode 1, one shot
        TIMER_RATEGEN   = 0x04, // mode 2, rate generator
        TIMER_SQWAVE    = 0x06, // mode 3, square wave
        TIMER_SWSTROBE  = 0x08, // mode 4, s/w triggered strobe
        TIMER_HWSTROBE  = 0x0A, // mode 5, h/w triggered strobe
        TIMER_LATCH     = 0x00, // Latch counter for reading
        TIMER_LSB       = 0x10, // r/w counter LSB
        TIMER_MSB       = 0x20, // r/w counter MSB
        TIMER_16BIT     = 0x30, // r/w counter 16 bits, LSB first
        TIMER_BCD       = 0x01, // Count in BCD
    };

    /************************************************************************************************************/

    // Change this if needed.
    const int TIMER_FREQ = 100; // in Hz

    // Physicial timer's frequency.
    const int TIMER_PHYSFREQ = 1193180; // don't change this! */

    const int HZ = (TIMER_PHYSFREQ / 10000);

    /************************************************************************************************************/

    void tick(struct regs* r)
    {
        UNUSED(r);
        printf("Tick\n");
    }

    void set_timer_freq(int freq)
    {
        int divis;

        /*
         * This unusual operation provides us with an extra bit we can use to
         * round the integer division up or down.  Correctly rounding this will
         * grant us a bit more accuracy with our timer's book keeping.
         */
        divis = (TIMER_PHYSFREQ << 1) / freq;
        divis = (divis >> 1) + (divis & 1);

        stop_interrupts();

        bus pit(TIMER_PORTBASE, 4);

        /* Adjust the clock rate */
        pit.byte(TIMER_MODE, TIMER_SEL0 | TIMER_16BIT | TIMER_SQWAVE);
        pit.byte(TIMER_COUNTER0, divis & 0xFF);
        pit.byte(TIMER_COUNTER0, (divis >> 8) & 0xFF);

        //s_freq = freq;
        //s_freq_cnt = divis ? divis : 0xFFFF;

        start_interrupts();
    }
}

/********************************************************************************************************************/

void init_timer(void)
{
    set_timer_freq(TIMER_FREQ);

    set_isr_callback(32, tick);

    printf("Timer initialized.\n");
}

/********************************************************************************************************************/
