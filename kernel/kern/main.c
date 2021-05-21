/*************************************************************************/
/*
 * $Id: main.c 187 2011-08-27 16:27:33Z kfiresun $
 */
/*************************************************************************/

#include "cdefs.h"
#include "multboot.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "ktime.h"
#include "paging.h"
#include "kheap.h"
#include "process.h"

//#include "ata.h"

/*************************************************************************/

void pmain(void);

void read_disk_info(multiboot_t *mbd);

extern void force_crash(void);

void test_stack_trace(void);

static const uint32_t *start_ebp;

/*************************************************************************/
/*
 * The kernel's main entry point.
 */
void kmain(multiboot_t *mbd, uintptr_t stackPtr)
{
    start_ebp = (uint32_t *)read_ebp();

    uint32_t cs = read_cs();
    uint32_t ip = (uint32_t)(kmain);
    uint32_t ss = read_ss();
    uint32_t sp = read_esp();

    // Should be moved to a driver section
    init_display();

    kprintf("CODE : %p:%p\n", cs, ip);
    kprintf("STACK: %p:%p\n", ss, sp);
    kprintf("STACK START: %p\n", stackPtr + 0x4000);
    kprintf("STACK END: %p\n", stackPtr);

    init_descriptor_tables();

    /* Interrupts should now be safe! */
    start_interrupts();

    init_paging(mbd);
    init_scheduler();
    init_timer();

    // Should be a driver
    init_keyboard();

    read_disk_info(mbd);

    // Debugging test
    /*
    kprintf("kmain() == %p\n", kmain);
    kprintf("test_stack_trace() == %p\n", test_stack_trace);
    kprintf("stack_trace() == %p\n", stack_trace);
    test_stack_trace();

    force_crash();
    */

    /* Start preemptive kernel thread. */
    create_process(pmain);

    /* After our first context switch, the code below will stop running. */

    for (;;)
    {
        asm volatile("pause"::);
    }
}

/*************************************************************************/
/*
 * Preemptive main
 */
void pmain(void)
{
    create_process(update_debug_sections);

    kprintf("Kernel loaded\n");

    for (;;)
    {
        asm volatile("pause"::);
    }
}

/*************************************************************************/

void read_disk_info(multiboot_t *mbd)
{
    int boot_dev;
    bool_t isHDD;
    int i;

    if ((mbd->flags & MB_FLAG_BOOTDEV) == 0)
    {
        kprintf("WARNING: UNKNOWN BOOT DEVICE!\n");
        return;
    }

    boot_dev = mbd->boot_device[3];

    isHDD = ((boot_dev & 0x80) != 0);
    boot_dev &= ~0x80;

    kprintf("Boot device: %s %d(", isHDD ? "HDD" : "FDD", boot_dev);

    for (i = 2; i >= 0; --i)
    {
        if (mbd->boot_device[i] == 0xFF)
            break;

        if (i != 2)
            kprintf(", ");

        kprintf("%d", i);
    }

    kprintf(")\n");
}

/*************************************************************************/
/*
 * Display a "STOP ERROR" message and halt the system.
 */
void kstop(const char *msg)
{
    //blank_screen();

    putstr("STOP ERROR: ");
    putstr(msg);
    putstr("\n\nTHE SYSTEM HAS BEEN HALTED");

    /* Die */
    khalt();
}

/*************************************************************************/

void stack_trace(uintptr_t stack)
{
    (void)(stack);
    uint32_t *ebp = (uint32_t *)read_ebp();
    uint32_t eip; //, *args;

    kprintf("Stack Trace:\n");

    for (;;)
    {
        eip = ebp[1];

        if (eip == 0)
        {
            kprintf("END OF STACK TRACE: EIP == 0\n");
            break;
        }

        ebp = (uint32_t *)ebp[0];
        //args = (uint32_t *)ebp[2];
        kprintf("  %p\n", eip);

        if (ebp >= start_ebp)
        {
            // Past kmain()
            kprintf("END OF STACK TRACE\n");
            break;
        }
    }
}

/*************************************************************************/

void test_stack_trace(void)
{
    stack_trace(read_ebp());
}

/*************************************************************************/

#if defined(__cplusplus)

extern "C" void __cxa_pure_virtual(void)
{
    // Die a horible ugly death.
    kstop("Pure virtual call");
}

/*
 * TODO: Put in exception handling functions here.
 */

#endif /* __cplusplus */

/*************************************************************************/
