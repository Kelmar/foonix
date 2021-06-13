/********************************************************************************************************************/

#include "asm.h"
#include "cpudefs.h"

/********************************************************************************************************************/

#define ISR_NO_PARAM(VAL) \
    _LABEL(_isr ## VAL);  \
    cli;                  \
    pushl $0;             \
    pushl $VAL;           \
    jmp  .isr_stub

#define ISR_ONE_PARAM(VAL)  \
    _LABEL(_isr ## VAL);    \
    cli;                    \
    pushl $VAL;             \
    jmp  .isr_stub

#define IRQ(VAL)              \
    _LABEL(_irq ## VAL);      \
    cli;                      \
    pushl $0;                 \
    pushl $(VAL + IRQ_START); \
    jmp  .isr_stub

/********************************************************************************************************************/

ISR_NO_PARAM(0)	    /* Divide by zero */
ISR_NO_PARAM(1)	    /* Debug */
ISR_NO_PARAM(2)	    /* NMI */
ISR_NO_PARAM(3)	    /* Breakpoint */
ISR_NO_PARAM(4)	    /* Into detected overflow */
ISR_NO_PARAM(5)	    /* Out of bounds */
ISR_NO_PARAM(6)	    /* Invalid Opcode */
ISR_NO_PARAM(7)	    /* No FPU */
ISR_ONE_PARAM(8)    /* Double fault */
ISR_NO_PARAM(9)	    /* FPU segment overrun */
ISR_ONE_PARAM(10)   /* TSS */
ISR_ONE_PARAM(11)   /* Segment not present */
ISR_ONE_PARAM(12)   /* Stack fault */
ISR_ONE_PARAM(13)   /* GPF */
ISR_ONE_PARAM(14)   /* Page fault */
ISR_NO_PARAM(15)    /* Unknown iterrupt */
ISR_NO_PARAM(16)    /* FPU fault */
ISR_NO_PARAM(17)    /* Alignment check (486+) */
ISR_NO_PARAM(18)    /* Machine check (586+) */
	
ISR_NO_PARAM(19)    /* Reserved */
ISR_NO_PARAM(20)    /* Reserved */
ISR_NO_PARAM(21)    /* Reserved */
ISR_NO_PARAM(22)    /* Reserved */
ISR_NO_PARAM(23)    /* Reserved */
ISR_NO_PARAM(24)    /* Reserved */
ISR_NO_PARAM(25)    /* Reserved */
ISR_NO_PARAM(26)    /* Reserved */
ISR_NO_PARAM(27)    /* Reserved */
ISR_NO_PARAM(28)    /* Reserved */
ISR_NO_PARAM(29)    /* Reserved */
ISR_NO_PARAM(30)    /* Reserved */
ISR_NO_PARAM(31)    /* Reserved */

IRQ(0)
IRQ(1)
IRQ(2)
IRQ(3)
IRQ(4)
IRQ(5)
IRQ(6)
IRQ(7)
IRQ(8)
IRQ(9)
IRQ(10)
IRQ(11)
IRQ(12)
IRQ(13)
IRQ(14)
IRQ(15)

/********************************************************************************************************************/

.isr_stub:
    /* Push registers for core dump. */
    pusha
    
    push %ds
    push %es
    push %fs
    push %gs
    
    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    /* We push a pointer to the structure we just filled out on the stack. */
    mov %esp, %eax
    push %eax
    call handle_isr /* call C code */
    
    pop %eax /* remove pointer */
    
    /* Restore registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp /* Clean up the pushed error code. */
    iret

/********************************************************************************************************************/
/*
 * Load an interrupt descriptor table:
 * void load_idt(struct idt_ptr *)
 */
_LABEL(load_idt)
    movl 4(%esp), %eax
    lidt (%eax)
    ret

/********************************************************************************************************************/
