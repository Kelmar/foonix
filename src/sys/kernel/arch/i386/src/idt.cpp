/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <kernel/debug.h>
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
        uint8_t  level   : 2; // Privilege level
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

    //int s_exception_depth = 0;

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
        /* 15 */ "Unknown Interrupt",
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

    for (int i = 0; i < IDT_ENTRIES; ++i)
        set_idt_entry(i, vectors[i], false, 0x08, 0);

    //set_idt_entry(0xF0, isr_call, true, 8, 0);

    load_idt(&s_idtp);
}

/********************************************************************************************************************/

void dump_regs(const struct regs *r)
{
    Debug::PrintF("CPU DUMP:\n");
    Debug::PrintF("EAX: %p  EBX: %p  ECX: %p  EDX: %p\n", r->eax, r->ebx, r->ecx, r->edx);
    Debug::PrintF("EDI: %p  ESI: %p  EBP: %p  ESP: %p\n", r->edi, r->esi, r->ebp, r->esp);
    Debug::PrintF(" GS: %p   FS: %p   ES: %p   DS: %p\n", r->gs, r->fs, r->es, r->ds);
    Debug::PrintF(" CS: %p  EIP: %p   SS: %p  ESP: %p\n", r->cs, r->eip, r->ss, r->useresp);
    //Debug::PrintF("CR0: %p  CR2: %p  CR3: %p  CR4: %p\n", read_cr0(), read_cr2(), read_cr3(), read_cr4());
    Debug::PrintF("FLG: %p\n", r->eflags);
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

    //terminal_clear();
    Debug::PrintF("Exception (%02X): %s\n", r->int_no, msg);
    Debug::PrintF("Error code: %d\n\n", r->err_code);

    dump_regs(r);
    //stack_trace(r->ebp);

    //puts("\nSystem Halted.");
    Debug::PrintF("\nSystem Halted.");
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

// TODO: Put into a proper header.
void pic_send_eoi(int irq_no);

/********************************************************************************************************************/
/*
 * Handles calling the correct callback.
 *
 * This is called by our assembly code, which is inturn called when an
 * interrupt is triggered.
 */
extern "C" void handle_isr(regs* r)
{

    //Debug::PrintF("Interrupt %d\n", r->int_no);

    int irq_no = r->int_no - 32;
    bool is_irq = ((irq_no >= 0) && (irq_no < MAX_IRQS));

/*
    if (++s_exception_depth > 1)
        panic("Caught nested exceptions.");
*/

    /*
     * Would like to rethink this a bit.  Might be useful to beable to have multiple handlers on a single IRQ
     * and it would also would be nice to have the PIC code to be able to actually detect (or receive) when
     * a hardware IRQ is actually handled or not.
     *
     * If it isn't handled, we would probably like for it to mask that bit off and prevent further interrupts
     * on that IRQ unless explicitly enabled by something else.
     *
     * We also should investigate in APIC (Advanced Programmable Interrupt Controller) and APCI (???)
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

    if (is_irq)
        pic_send_eoi(irq_no);
}

/********************************************************************************************************************/
