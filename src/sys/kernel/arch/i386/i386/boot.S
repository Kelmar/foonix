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
    /* Boot pages */
    .align 4096
    LABEL(boot_page_identity)
    .skip 4096

    .align 4096
    LABEL(boot_page_kernel)
    .skip 4096

    .align 4096
    LABEL(boot_page_directory)
    .skip 4096

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

.extern _kernel_phys_start
.extern _kernel_end

.extern _paged_start

.global paging_init

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

    /* Setup boot info for our preinit call */
    movl %eax, VIRT_2_PHYS(g_bootMagic)
    movl %ebx, VIRT_2_PHYS(g_bootInfoPtr)

    /* Preserve EAX and EBX which hold boot info from multiboot */
    pushl %ebx
    pushl %eax

    /* EAX and EBX as arguments to preinit */
    pushl %ebx
    pushl %eax

    /* Call new_preinit to setup all the things needed by a proper call to main(). */
    call new_preinit

    /* Setup directory and page tables */

    /* Setup lower 4MB identity pages. */
    movl $VIRT_2_PHYS(boot_page_identity), %edi
    movl $1024, %ecx

    movl $3, %eax /* Mark page as writable and present. */

.identity_page_loop:
    movl %eax, (%edi)
    addl $0x1000, %eax
    addl $4, %edi
    loop .identity_page_loop

    /* Setup pages at 0xC0000000*/
    movl $VIRT_2_PHYS(boot_page_kernel), %edi
    movl $1024, %ecx
    movl $0, %esi

    /* Loop start */
.kernel_page_loop:
    /* Reached end of kernel? */
    cmpl $VIRT_2_PHYS(_kernel_end), %esi
    jge .kernel_page_done

    cmpl $_kernel_phys_start, %esi
    jge .enable_page

    /* Haven't reached the kernel yet; make the entry blank. */
    xorl %eax, %eax
    jmp .set_page_entry

.enable_page:
    /* Map the kernel page. */
    movl %esi, %eax
    orl  $0x03, %eax

.set_page_entry:
    /* Set entry */
    movl %eax, (%edi)

    /* Loop tail */
    addl $0x1000, %esi
    addl $4, %edi
    loop .kernel_page_loop

.kernel_page_done:
    /* Loop complete, blank the remaining pages and our directory */
    addl $1024, %ecx /* Include directory in blank */
    shrl $4, %ecx /* Storing DWORDS */

    xorl %eax, %eax
    rep  stosl /* stosd */

    /* Temporary debugging, add text video memory map */
    /* movl $0x000B8003, VIRT_2_PHYS(boot_page_identity) + VIDEO_PAGE_OFFSET */

    movl $VIRT_2_PHYS(boot_page_identity), %eax
    orl  $0x03, %eax

    /* Add identity map */
    movl %eax, VIRT_2_PHYS(boot_page_directory)

    /* Add kernel virtual map */
    movl $VIRT_2_PHYS(boot_page_kernel), %eax
    orl  $0x03, %eax
    movl %eax, VIRT_2_PHYS(boot_page_directory) + KERNEL_PAGE_OFFSET

break_0001:
    /* Restore the EAX and EBX registers */
    popl %eax
    popl %ebx

    /* Setup directory address in CR3 */
    movl $VIRT_2_PHYS(boot_page_directory), %ecx
    movl %ecx, %cr3

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
    // Preserve eax for selector setup.
    movl %eax, %edx

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

    /* Setup boot info for our preinit call */
    movl %eax, g_bootMagic
    movl %ebx, g_bootInfoPtr

    call preinit /* Initialize CPU & VM first */

    /* Call C++ static constructors */
    /* call __runtime_init */

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
