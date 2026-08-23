/********************************************************************************************************************/
/*
 * Paged mode entry point.
 *
 * Called from start.S
 */
/********************************************************************************************************************/

#include "asm.h"

#define KERNEL_PAGE_OFFSET (((KERNEL_OFFSET) >> 22) * 4)
#define VIDEO_PAGE_OFFSET (((VIDEO_OFFSET) >> 12) * 4)

/********************************************************************************************************************/

.section .bss, "aw", @nobits
# Reserve initial kernel stack space.
    .align 16
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

.global stack_top

/********************************************************************************************************************/

.section .data
    /* System x64 global descriptor table */
    .align 8
LABEL(g_gdtEntries)
    .quad 0x0000000000000000 /* NULL descriptor */
    .quad 0x00CF9A000000FFFF /* Code segment (Kernel) */
    .quad 0x00CF92000000FFFF /* Data segment (Kernel) */
    .quad 0x00CFFA000000FFFF /* Code segment (User) */
    .quad 0x00CFF2000000FFFF /* Data segment (User) */
    .quad 0x0000000000000000 /* TSS */

    .align 4
LABEL(g_gdtPtr)
    .word . - g_gdtEntries - 1
    .long g_gdtEntries

/********************************************************************************************************************/

.section .text

/********************************************************************************************************************/

/*
 * This is the very first thing that the bootloader calls for our kernel.
 */

.global _start
.type _start, @function
_start:
    /* Get some CPU flags into a known state. */
    cli
    cld

    /* Setup temporary stack (using physical address) */
    movl $VIRT_2_PHYS(stack_top), %esp
    movl $VIRT_2_PHYS(stack_top), %ebp

    /* EAX and EBX as arguments to preinit */
    pushl %ebx
    pushl %eax

    /* Call preinit to setup all the things needed by a proper call to main(). */
    call preinit

    /* Turn on paging and write protection */
paging_init:
    movl %cr0, %ecx
    orl  $0x80010000, %ecx
    movl %ecx, %cr0

    /* Load a GDT that we know the location of. */
    lgdt g_gdtPtr

    /* Long jump into our _paged_start function in boot.S */
    ljmp $(1 << 3), $_paged_start /* Load CS register and switch to paged mode */

/********************************************************************************************************************/
/*
 * From here on out we use virtual addresses as the C/C++ compiler expects them to be.
 * (In other words, it is now safe to call C/C++ functions.)
 */
.section .text
.global _paged_start
.type _paged_start, @function
_paged_start:
    /* Load values for kernel selectors */
    movw $(2 << 3), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    /* Blank fs and gs selectors */
    xorw %ax, %ax
    movw %ax, %fs
    movw %ax, %gs

    movl %edx, %eax

    /* Setup temporary stack (using virtual addresses) */
    movl $stack_top, %esp
    movl $stack_top, %ebp

    # Call the kernel proper
    call kmain

    cli /* Disable interrupts if they're still active */
    jmp .L_halt

.L_halt:
    # Halt machine.
    hlt
    pause
    jmp .L_halt

/********************************************************************************************************************/
