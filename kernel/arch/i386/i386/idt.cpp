/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <kernel/flow.h>

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

#define IDT_ENTRIES 256
#define MAX_IRQS 16

/********************************************************************************************************************/
namespace
{
    /*
     * Interrupt descriptor table entry.
     */
    struct idt_entry_t
    {
        uint16_t base_low;      /* Base address low */
        uint16_t selector;      /* Kernel segment selector */
        uint8_t  reserved;      /* Reserved, always zero */
        uint8_t  flags;         /* Flags */
        uint16_t base_hi;       /* Base address high */
    } __attribute__((packed));

    /*
     * Interrupt descriptor table pointer.
     */
    struct idt_ptr_t
    {
        uint16_t    limit;
        idt_entry_t* base;
    } __attribute__((packed));

    typedef void (*isr_stub_t)(void);

    /************************************************************************************************************/

    idt_entry_t s_idt[IDT_ENTRIES];
    idt_ptr_t s_idtp;

    isr_handler_t s_isr_callbacks[IDT_ENTRIES];

    int s_exception_depth = 0;

    const char* exception_messages[] =
    {
        /*  0 */ "Divide by zero",
        /*  1 */ "Debug",
        /*  2 */ "Non Maskable Interrupt",
        /*  3 */ "Breakpoint",
        /*  4 */ "Into Detected Overflow",
        /*  5 */ "Out of Bounds",
        /*  6 */ "Invalid Opcode",
        /*  7 */ "No FPU",
        /*  8 */ "Double Fault",
        /*  9 */ "FPU Segment Overrun",
        /* 10 */ "Bad TSS",
        /* 11 */ "Segment Not Present",
        /* 12 */ "Stack Fault",
        /* 13 */ "General Protection Fault",
        /* 14 */ "Page Fault",
        /* 15 */ "Unknown Inerrupt",
        /* 16 */ "FPU Fault",
        /* 17 */ "Alignment Check",
        /* 18 */ "Machine Check",
        /* 19 */ "Reserved 19",
        /* 20 */ "Reserved 20",
        /* 21 */ "Reserved 21",
        /* 22 */ "Reserved 22",
        /* 23 */ "Reserved 23",
        /* 24 */ "Reserved 24",
        /* 25 */ "Reserved 25",
        /* 26 */ "Reserved 26",
        /* 27 */ "Reserved 27",
        /* 28 */ "Reserved 28",
        /* 29 */ "Reserved 29",
        /* 30 */ "Reserved 30",
        /* 31 */ "Reserved 31",
        0 /* fence post */
    };

    /************************************************************************************************************/
    // Assembly ISR and IRQ stubs

#define ISR(X) extern "C" void _isr ## X(void)
#define IRQ(X) extern "C" void _irq ## X(void)

    ISR(0);
    ISR(1);
    ISR(2);
    ISR(3);
    ISR(4);
    ISR(5);
    ISR(6);
    ISR(7);
    ISR(8);
    ISR(9);
    ISR(10);
    ISR(11);
    ISR(12);
    ISR(13);
    ISR(14);
    ISR(15);
    ISR(16);
    ISR(17);
    ISR(18);
    ISR(19);
    ISR(20);
    ISR(21);
    ISR(22);
    ISR(23);
    ISR(24);
    ISR(25);
    ISR(26);
    ISR(27);
    ISR(28);
    ISR(29);
    ISR(30);
    ISR(31);

    IRQ(0);
    IRQ(1);
    IRQ(2);
    IRQ(3);
    IRQ(4);
    IRQ(5);
    IRQ(6);
    IRQ(7);
    IRQ(8);
    IRQ(9);
    IRQ(10);
    IRQ(11);
    IRQ(12);
    IRQ(13);
    IRQ(14);
    IRQ(15);

#undef IRQ
#undef ISR
}

/********************************************************************************************************************/

isr_handler_t set_isr_callback(uint8_t isr_no, isr_handler_t callback)
{
    isr_handler_t rval = s_isr_callbacks[isr_no];

    s_isr_callbacks[isr_no] = callback;

    return rval;
}

/********************************************************************************************************************/

void init_idt(void)
{
    // Set all callbacks to NULL
    memset(s_isr_callbacks, 0, sizeof(isr_handler_t) * IDT_ENTRIES);
    memset(s_idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

    // Initialize the IDT
    s_idtp.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    s_idtp.base = s_idt;

    bus master_pic((void*)0x0020, 2);
    bus slave_pic((void*)0x00A0, 2);

    /*
     * These magic numbers tell the PICs to remap what interrupts are fired when an IRQ is triggered.
     *
     * TODO: Get details about reprogramming the PICs.
     */
    master_pic.write1(0, 0x11);
    slave_pic .write1(0, 0x11);
    master_pic.write1(1, 0x20); // IRQ_START?
    slave_pic .write1(1, 0x28); // IRQ_START + 8?
    master_pic.write1(1, 0x04);
    slave_pic .write1(1, 0x02);
    master_pic.write1(1, 0x01);
    slave_pic .write1(1, 0x01);
    master_pic.write1(1, 0);
    slave_pic .write1(1, 0);
}

/********************************************************************************************************************/
/*
 * Handler for panic conditions.
 */
static void panic_handler(struct regs* r)
{
    const char* msg = "Unknown Exception";

    if (r->int_no < 32)
        msg = exception_messages[r->int_no];

    /*
     * Display our own little BSOD and halt the system.
     */
     //blank_screen();
    printf("Exception (%02X): %s\n", r->int_no, msg);
    printf("Error code: %d\n\n", r->err_code);

    //dump_regs(r);
    //stack_trace(r->ebp);

    puts("\nSystem Halted.");
    khalt();
}

/********************************************************************************************************************/
/*
 * Default interrupt handler
 */
static void default_handler(struct regs* r)
{
    UNUSED(r);
}

/********************************************************************************************************************/
/*
 * Handles calling the correct callback.
 *
 * This is called by our assembly code, which is inturn called when an
 * interrupt is triggered.
 */
extern "C" void handle_isr(regs* r)
{
    //char buf[16];
    //int i;

    int irq_no = r->int_no - 32;
    int is_irq = ((irq_no >= 0) && (irq_no < MAX_IRQS));

    if (++s_exception_depth > 1)
        panic("Caught nested exceptions.");

    isr_handler_t cb = s_isr_callbacks[r->int_no];

    if (cb == nullptr)
    {
        if (!is_irq && (r->int_no < 32))
            panic_handler(r);
        else
            default_handler(r);
    }
    else
    {
        /* Call the callback */
        cb(r);
    }

    //if (r->int_no != 32)
    //{
    //    i = snkprintf(buf, sizeof(buf), "INT: %02X", r->int_no);
    //}
    //else
    //    i = 0;

    //if (i != 0)
    //    set_debug_section_info(s_debsect, buf, i);

    if (is_irq)
    {
        bus master_pic((void*)0x0020, 2);

        /*
         * If IRQ 8-15, we need to send an EOI to the slave controller.
         */
        if (irq_no >= 8)
        {
            bus slave_pic((void*)0x00A0, 2);
            slave_pic.write1(0, 0x20);
        }

        /*
         * In all cases we need to send an EOI to the master controller.
         */
        master_pic.write1(0, 0x20);
    }

    --s_exception_depth;
}

/********************************************************************************************************************/
