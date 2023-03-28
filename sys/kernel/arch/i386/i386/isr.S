/********************************************************************************************************************/

#include "asm.h"
#include "cpudefs.h"

/********************************************************************************************************************/

.globl isr_stub
isr_stub:
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
