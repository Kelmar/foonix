/********************************************************************************************************************/
/*
 * Long mode entry point.
 *
 * Called by start.S
 */
/********************************************************************************************************************/

#include "asm.h"

/********************************************************************************************************************/

# Reserve initial kernel stack space.
.section .bss, "aw", @nobits
    .align 16
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

.global stack_top
.global prepreinit

/********************************************************************************************************************/

.section .text

.code64

.global _x64_start
.type _x64_start, @function
_x64_start:
    # Need to adjust segment registers before we can use memory.
    xorw %dx, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %fs
    movw %dx, %gs
    movw %dx, %ss

    # Setup temporary stack (using virtual addresses)
    movq $stack_top, %rsp
    movq $stack_top, %rbp

    # Note that rdi and rsi were loaded from start.S prior to the long jump.
    # This has our multiboot parameters in them.

    call preinit /* Initialize CPU & VM first */

    call kmain

    cli /* Ensure interrupts are disabled if we get here. */

1:
    # Halt machine.
    hlt
    pause
    jmp 1b

/********************************************************************************************************************/
