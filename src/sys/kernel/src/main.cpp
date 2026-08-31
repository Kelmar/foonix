/********************************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "cpu.h"

#include <kernel/arch/dconsole.h>
#include <kernel/debug.h>
#include <kernel/console.h>
#include <kernel/tty.h>

#include <kernel/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/interrupt.h>

#include <kernel/vm.h>
#include <kernel/kalloc.h>

//#include "kernio.h"
//#include "ktime.h"
#include "paging.h"
//#include "kheap.h"
//#include "process.h"

//#include "ata.h"

/********************************************************************************************************************/
/**
 * @brief Global kernel arguments structure.
 */
KernelArgs g_kernelArguments;

namespace cpu
{
    InterruptStack interrupt_stack;
}

/********************************************************************************************************************/

#if 0
void pmain(void);

void read_disk_info(multiboot_t *mbd);
#endif

/********************************************************************************************************************/
/*
 * The kernel's main entry point.
 */
extern "C" void kmain(void)
{
    Debug::PrintF("ENTER: kmain()\r\n");

    new (&cpu::interrupt_stack) cpu::InterruptStack();
    new (&g_kernelArguments) KernelArgs();
    new (&console) Console();

    arch::Init(&g_kernelArguments);

    vmm::Init();

    memory::init_kalloc();

#if 0
    init_scheduler();

    // Should be a driver
    init_keyboard();

    read_disk_info(mbd);

    /* Start preemptive kernel thread. */
    create_process(pmain);

    /* After our first context switch, the code below will stop running. */
#endif

    Debug::PrintF("g_kernelArguments = %p\r\n", &g_kernelArguments);
    Debug::PrintF("console = %p\r\n", &console);

    Debug::shell();
}

/********************************************************************************************************************/

#if 0

/********************************************************************************************************************/
/*
 * Preemptive main
 */
void pmain(void)
{
    create_process(update_debug_sections);

    kprintf("Kernel loaded\n");

    for (;;)
        cpu::pause();
}

/********************************************************************************************************************/

void read_disk_info(multiboot_t *mbd)
{
    int boot_dev;
    bool_t isHDD;
    int i;

    if ((mbd->flags & MB_FLAG_BOOTDEV) == 0)
    {
        Debug::PrintF("WARNING: UNKNOWN BOOT DEVICE!\n");
        return;
    }

    boot_dev = mbd->boot_device[3];

    isHDD = ((boot_dev & 0x80) != 0);
    boot_dev &= ~0x80;

    Debug::PrintF("Boot device: %s %d(", isHDD ? "HDD" : "FDD", boot_dev);

    for (i = 2; i >= 0; --i)
    {
        if (mbd->boot_device[i] == 0xFF)
            break;

        if (i != 2)
            Debug::PrintF(", ");

        Debug::PrintF("%d", i);
    }

    Debug::PrintF(")\n");
}

/********************************************************************************************************************/
#endif