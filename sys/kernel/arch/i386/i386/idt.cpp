/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <kernel/flow.h>
#include <kernel/tty.h>

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

namespace
{
    const int IDT_ENTRIES = 256;
    const int MAX_IRQS = 16;

    //const uint8_t TYPE_TSS  = 0x09;
    const uint8_t TYPE_INT  = 0x0E;
    const uint8_t TYPE_TRAP = 0x0F;

    // Interrupt descriptor table entry.
    struct idt_entry_t
    {
        uint16_t base_low;    // Base address low
        uint16_t selector;    // Kernel segment selector
        uint8_t  reserved;    // Reserved, always zero

        uint8_t  type    : 4; // 9 = TSS, E = 32bit Int Gate, F = 32bit Trap Gate
        uint8_t  storage : 1; // Always zero
        uint8_t  level   : 2; // Privelege level
        uint8_t  present : 1; // Always 1

        uint16_t base_hi;     // Base address high
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

    /* Defined in isr.s */
    extern "C" void load_idt(idt_ptr_t*);

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

    void set_idt_entry(int idx, isr_stub_t base, bool trap, int selector, int level)
    {
        uintptr_t b = (uintptr_t)base;

        idt_entry_t* idt = &s_idt[idx];

        idt->base_low = (uint16_t)(b & 0xFFFF);
        idt->base_hi = (uint16_t)((b >> 16) & 0xFFFF);
        idt->selector = selector;
        idt->reserved = 0;
        idt->type = trap ? TYPE_TRAP : TYPE_INT;
        idt->storage = 0;
        idt->level = level;
        idt->present = 1;
    }

    /************************************************************************************************************/
}

// Assembly ISR and IRQ stubs
extern "C" isr_stub_t vectors[IDT_ENTRIES];

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
    master_pic.byte(0, 0x11);
    slave_pic .byte(0, 0x11);
    master_pic.byte(1, 0x20); // vector: IRQ_START
    slave_pic .byte(1, 0x28); // vector: IRQ_START + 8
    master_pic.byte(1, 0x04);
    slave_pic .byte(1, 0x02);
    master_pic.byte(1, 0x01);
    slave_pic .byte(1, 0x01);
    master_pic.byte(1, 0);
    slave_pic .byte(1, 0);

    //master_pic.byte(1, 0xFD);
    //slave_pic.byte(1, 0xFF);

    for (int i = 0; i < IDT_ENTRIES; ++i)
        set_idt_entry(i, vectors[i], false, 0x08, 0);

    //set_idt_entry(0xF0, isr_call, true, 8, 0);

    load_idt(&s_idtp);

    start_interrupts();
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

    // Display our own little BSOD and halt the system.

    terminal_clear();
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

extern "C" uint8_t inb(io_port_t port);
extern "C" void outb(io_port_t port, uint8_t byte);

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

    //printf("Interrupt %d\n", r->int_no);

    int irq_no = r->int_no - 32;
    bool is_irq = ((irq_no >= 0) && (irq_no < MAX_IRQS));

/*
    if (++s_exception_depth > 1)
        panic("Caught nested exceptions.");
*/

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
        // Call the callback
        cb(r);
    }

    //if (r->int_no != 32)
    //{
    //    i = snprintf(buf, sizeof(buf), "INT: %02X", r->int_no);
    //}
    //else
    //    i = 0;

    //if (i != 0)
    //    set_debug_section_info(s_debsect, buf, i);

    if (is_irq)
    {
        //bus master_pic((void*)0x0020, 2);

        if (irq_no == 1)
        {
            uint8_t scan = inb((uint16_t)0x60);
            (void)(scan);
        }

        // If IRQ 8-15, we need to send an EOI to the slave controller.
        if (irq_no >= 8)
        {
            //bus slave_pic((void*)0x00A0, 2);
            //slave_pic.byte(0, 0x20);
            outb((uint16_t)0x00A0, 0x20);
        }

        // In all cases we need to send an EOI to the master controller.
        //master_pic.byte(0, 0x20);
        outb((uint16_t)0x0020, 0x20);
    }

    --s_exception_depth;
}

/********************************************************************************************************************/
