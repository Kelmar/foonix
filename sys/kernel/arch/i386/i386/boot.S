/*************************************************************************/
/*
 * Paged mode entry point.
 *
 * Called by multiboot.S
 */
/*************************************************************************/

#include "asm.h"

/*************************************************************************/

# Reserve initial kernel stack space.
.section .bss, "aw", @nobits
    .align 16
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

/*************************************************************************/

.section .text, "a"
.global _paged_start
.type _paged_start, @function
_paged_start:
    /* Setup temporary stack */
    movl $stack_top, %esp
    movl $stack_top, %ebp

    /* Setup boot info for our preinit call */
    movl %edx, g_BootMagic
    movl %ebx, g_Multiboot

    call  preinit /* Initialize CPU & VM first */

    /* Call static constructors */
    call  _init

    # Call the kernel proper
    call  kmain

    cli /* Disable interrupts if they're still active */
    jmp   .L_halt

.L_halt:
    # Halt machine.
    hlt
    pause
    jmp   .L_halt

/*************************************************************************/
