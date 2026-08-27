/********************************************************************************************************************/

#include "asm.h"
#include "cpudefs.h"

/********************************************************************************************************************/

.globl isr_stub
isr_stub:
    /* Push registers for core dump. */
    pusha
    
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    
    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    /* We push a pointer to the structure we just filled out on the stack. */
    pushl %esp
    call handle_isr /* call C code */

    add $4,%esp /* Remove pointer */
    
    /* Restore registers */
    popl %gs
    popl %fs
    popl %es
    popl %ds
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
