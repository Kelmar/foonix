/*************************************************************************/
/*
 * $Id: desctab.c 64 2010-09-03 03:34:41Z kfiresun $
 */
/*************************************************************************/

#include "cdefs.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "cpu.h"
#include "kernio.h"

/*************************************************************************/

#define IDT_ENTRIES 256
#define GDT_ENTRIES 5
#define MAX_IRQS 16

/*************************************************************************/

/*
 * A global descriptor table entry.
 */
struct gdt_entry_TYPE
{
    uint16_t limit_low;	    /* Limit low part */
    uint16_t base_low;	    /* Base address lo part */
    uint8_t  base_middle;   /* Base address mid part */
    uint8_t  access;	    /* Access flags */
    uint8_t  granularity;   /* Limit granularity */
    uint8_t  base_high;	    /* Base address high */
} __attribute__((packed));

typedef struct gdt_entry_TYPE gdt_entry_t;

/*
 * Same as above, but broken into bit fields.
 *
 * Notes:
 * If the present flag is clear, then every field
 * except for type, sys_flag, priv_level, and present
 * are available for OS use.
 *
 * l_flag should always be zero for now. (Its for
 * writing mixed 32/64 bit code)
 */
struct gdt_entry2_TYPE
{
    uint16_t limit_low;       /* Limit 15:00 */
    uint16_t base_low;        /* Base 15:00 */
    uint8_t  base_middle;     /* Base 23:16 */

    uint8_t  type        : 4; /* Type of descriptor */
/* These are for when the sys_flag is set */
#define TYPE_ACCESSED         0x01 /* Set by CPU */

#define TYPE_DATA             0x00
#define TYPE_DATA_WRITABLE    0x02
#define TYPE_DATA_EXPAND_DOWN 0x04

#define TYPE_CODE             0x08
#define TYPE_CODE_READABLE    0x02
#define TYPE_CODE_CONFORMING  0x04

    uint8_t  sys_flag    : 1; /* System flag (0 = system, 1 = code or data) */
    uint8_t  priv_level  : 2; /* Privilage level */
    uint8_t  present     : 1; /* Present flag */

    uint8_t  limit_mid   : 4; /* Limit 19:16 */
    uint8_t  available   : 1; /* Available flag */
    uint8_t  l_flag      : 1; /* 64-bit code segment */
    uint8_t  op_size     : 1; /* Default operation size (0 = 16 bit; 1 = 32 */
    uint8_t  granularity : 1; /* Granularity flag */

    uint8_t  base_high;       /* Base 31:24 */

} __attribute__((packed));

typedef struct gdt_entry2_TYPE gdt_entry2_t;

/*
 * Global descriptor table pointer.
 */
struct gdt_ptr_TYPE
{
    uint16_t	limit;	    /* Number of entries */
    gdt_entry_t	*base;	    /* Pointer to table itself. */
} __attribute__((packed));

typedef struct gdt_ptr_TYPE gdt_ptr_t;

/*
 * Interrupt descriptor table entry.
 */
struct idt_entry_TYPE
{
    uint16_t base_low;	    /* Base address low */
    uint16_t selector;	    /* Kernel segment selector */
    uint8_t  reserved;	    /* Reserved, always zero */
    uint8_t  flags;	    /* Flags */
    uint16_t base_hi;	    /* Base address high */
} __attribute__((packed));

typedef struct idt_entry_TYPE idt_entry_t;

/*
 * Interrupt descriptor table pointer.
 */
struct idt_ptr_TYPE
{
    uint16_t	limit;
    idt_entry_t	*base;
} __attribute__((packed));

typedef struct idt_ptr_TYPE idt_ptr_t;

typedef void (*isr_stub_t)(void);

/*************************************************************************/

static gdt_entry_t s_gdt[GDT_ENTRIES];
static gdt_ptr_t s_gp;

static idt_entry_t s_idt[IDT_ENTRIES];
static idt_ptr_t s_idtp;

static isr_handler_t s_isr_callbacks[IDT_ENTRIES];

static int s_debsect = -1;
static int s_exception_depth = 0;

static char *exception_messages[] =
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

/*************************************************************************/
/* Assembly ISR & IRQ stubs */

#define ISR(VAL) extern void _isr ## VAL(void)

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

#undef ISR

#define IRQ(X) extern void _irq ## X(void)

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

/*************************************************************************/
/* Additional prototypes */

/* Defined in cpu.S */
void load_gdt(gdt_ptr_t *);

/* Defined in isr.s */
extern void load_idt(idt_ptr_t *);

static void init_gdt(void);
static void init_interrupts(void);

/*************************************************************************/
/*
 * Set's up an ISR callback function.
 */
isr_handler_t set_isr_callback(uint8_t isr_no, isr_handler_t callback)
{
    isr_handler_t rval = s_isr_callbacks[isr_no];

    s_isr_callbacks[isr_no] = callback;

    return rval;
}

/*************************************************************************/
/*
 * Initializes the descriptor tables.
 */
void init_descriptor_tables(void)
{
    s_debsect = get_debug_section();

    init_gdt();
    init_interrupts();
}

/*************************************************************************/
/*
 * Setup a descriptor in the GDT.
 */
static void gdt_set_gate(int num,
                         void *base, uint32_t limit,
                         uint8_t access, uint8_t gran)
{
    uintptr_t b = (uintptr_t)base;

    /* Setup the descriptor base address */
    s_gdt[num].base_low	    = b & 0xFFFF;
    s_gdt[num].base_middle  = (b >> 16) & 0xFF;
    s_gdt[num].base_high    = (b >> 24) & 0xFF;

    /* Setup the descriptor limits */
    s_gdt[num].limit_low    = limit & 0xFFFF;
    s_gdt[num].granularity  = (limit >> 16) & 0x0F;

    /* Finally, set up the access flag. */
    s_gdt[num].granularity |= gran & 0xF0;
    s_gdt[num].access	    = access;
}

#define ACCESS_MISSING 0x00
#define ACCESS_PRESENT 0x80
#define ACCESS_KERNEL  0x00
#define ACCESS_USER    0x60
#define ACCESS_GT_BIT  0x10 /* Don't know what this does... */
#define ACCESS_CODE    0x0A
#define ACCESS_DATA    0x02

/*************************************************************************/
/*
 * Should be called by init_descriptor_tables().  This will setup the special
 * GDT pointer, setup the first 3 entries in our GDT, and then finally call
 * gdt_flush() in our assembler file in order to tell the CPU where the new
 * GDT is and update the new segment registers.
 */
static void init_gdt(void)
{
    s_gp.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    s_gp.base = s_gdt;

    gdt_set_gate(0, 0, 0, 0, 0);		/* NULL sgement */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User mode code segment */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User mode data segment */

    /* Flush the old GDT and install the new one. */
    load_gdt(&s_gp);

    kprintf("GDT initialized\n");
}

/*************************************************************************/

static void idt_set_gate(uint8_t num, isr_stub_t base,
			 uint16_t sel, uint8_t flags)
{
    uintptr_t b = (uintptr_t)base;

    s_idt[num].base_low	= b & 0xFFFF;
    s_idt[num].base_hi	= (b >> 16) & 0xFFFF;
    s_idt[num].reserved	= 0;
    s_idt[num].selector	= sel;
    s_idt[num].flags	= flags;
}

/*************************************************************************/

static void init_interrupts(void)
{
    /* Set all callbacks to NULL */
    memset(s_isr_callbacks, 0, sizeof(isr_handler_t) * IDT_ENTRIES);
    memset(s_idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

    /* Initialize the IDT */
    s_idtp.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    s_idtp.base = s_idt;

    /*
     * These magic numbers tell the PICs to remap what interrupts are fired
     * when an IRQ is triggered.
     *
     * TODO: Get details about reprogramming the PICs.
     */
    bus_write_1((unsigned char *)0x20, 0, 0x11);
    bus_write_1((unsigned char *)0xA0, 0, 0x11);
    bus_write_1((unsigned char *)0x21, 0, 0x20); // IRQ_START?
    bus_write_1((unsigned char *)0xA1, 0, 0x28); // IRQ_START + 8?
    bus_write_1((unsigned char *)0x21, 0, 0x04);
    bus_write_1((unsigned char *)0xA1, 0, 0x02);
    bus_write_1((unsigned char *)0x21, 0, 0x01);
    bus_write_1((unsigned char *)0xA1, 0, 0x01);
    bus_write_1((unsigned char *)0x21, 0, 0x0);
    bus_write_1((unsigned char *)0xA1, 0, 0x0);

#define INST_ISR(VAL) idt_set_gate(VAL, _isr ## VAL, 0x08, 0x8E)

    INST_ISR(0);
    INST_ISR(1);
    INST_ISR(2);
    INST_ISR(3);
    INST_ISR(4);
    INST_ISR(5);
    INST_ISR(6);
    INST_ISR(7);
    INST_ISR(8);
    INST_ISR(9);
    INST_ISR(10);
    INST_ISR(11);
    INST_ISR(12);
    INST_ISR(13);
    INST_ISR(14);
    INST_ISR(15);
    INST_ISR(16);
    INST_ISR(17);
    INST_ISR(18);
    INST_ISR(19);
    INST_ISR(20);
    INST_ISR(21);
    INST_ISR(22);
    INST_ISR(23);
    INST_ISR(24);
    INST_ISR(25);
    INST_ISR(26);
    INST_ISR(27);
    INST_ISR(28);
    INST_ISR(29);
    INST_ISR(30);
    INST_ISR(31);

#undef INST_ISR

#define INST_IRQ(VAL) idt_set_gate(VAL + 32, _irq ## VAL, 0x08, 0x8E)

    INST_IRQ(0);
    INST_IRQ(1);
    INST_IRQ(2);
    INST_IRQ(3);
    INST_IRQ(4);
    INST_IRQ(5);
    INST_IRQ(6);
    INST_IRQ(7);
    INST_IRQ(8);
    INST_IRQ(9);
    INST_IRQ(10);
    INST_IRQ(11);
    INST_IRQ(12);
    INST_IRQ(13);
    INST_IRQ(14);
    INST_IRQ(15);

#undef INST_IRQ

    load_idt(&s_idtp);
    kprintf("IDT initialized\n");
}

/*************************************************************************/

void dump_regs(const struct regs *r)
{
    putstr("CPU DUMP:\n");
    kprintf("EAX: %p  EBX: %p  ECX: %p  EDX: %p\n",
	r->eax, r->ebx, r->ecx, r->edx);
    kprintf("EDI: %p  ESI: %p  EBP: %p  ESP: %p\n",
	r->edi, r->esi, r->ebp, r->esp);
    kprintf(" GS: %p   FS: %p   ES: %p   DS: %p\n",
	r->gs, r->fs, r->es, r->ds);
    kprintf(" CS: %p  EIP: %p   SS: %p  ESP: %p\n",
	r->cs, r->eip, r->ss, r->useresp);
    kprintf("CR0: %p  CR2: %p  CR3: %p  CR4: %p\n",
	read_cr0(), read_cr2(), read_cr3(), read_cr4());
    kprintf("FLG: %p\n",
	r->eflags);
}

/*************************************************************************/
/*
 * Handler for panic conditions.
 */
static void panic_handler(struct regs *r)
{
    char *msg = "Unknown Exception";

    if (r->int_no < 32)
	msg = exception_messages[r->int_no];

    /*
     * Display our own little BSOD and halt the system.
     */
    //blank_screen();
    kprintf("Exception (%02X): %s\n", r->int_no, msg);
    kprintf("Error code: %d\n\n", r->err_code);

    dump_regs(r);
    stack_trace(r->ebp);

    putstr("\nSystem Halted.");
    khalt();
}

/*************************************************************************/
/*
 * Default interrupt handler
 */
static void default_handler(struct regs *r)
{
    UNUSED(r);
}

/*************************************************************************/
/*
 * Handles calling the correct callback.
 *
 * This is called by our assembly code, which is inturn called when an
 * interrupt is triggered.
 */
void handle_isr(struct regs *r)
{
    char buf[16];
    int i;

    int irq_no = r->int_no - 32;
    int is_irq = ((irq_no >= 0) && (irq_no < MAX_IRQS));

    if (++s_exception_depth > 1)
    {
	kstop("Caught nested exceptions.");
    }

    isr_handler_t cb = s_isr_callbacks[r->int_no];

    if (cb == NULL)
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

    if (r->int_no != 32)
    {
	i = snkprintf(
	    buf, sizeof(buf),
	    "INT: %02X", r->int_no);
    }
    else
	i = 0;

    if (i != 0)
	set_debug_section_info(s_debsect, buf, i);

    if (is_irq)
    {
	/*
	 * If IRQ 8-15, we need to send an EOI to the slave controller.
	 */
	if (irq_no >= 8)
	    bus_write_1((unsigned char *)0xA0, 0, 0x20);

	/*
	 * In all cases we need to send an EOI to the master controller.
	 */
	bus_write_1((unsigned char *)0x20, 0, 0x20);
    }

    --s_exception_depth;
}

/*************************************************************************/
