/********************************************************************************************************************/
/*
 * This is our initial boot loader after stage 0 or 1.  We get called from 
 * everything here, EFI, multiboot and so on.
 *
 * This file establishes a few conventions to make sure we preserve some information:
 *
 * 1) We need to preserve the initial contents of EAX and EBX, which for multiboot has the multiboot information we
 *    need later in the C++ code.
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

/********************************************************************************************************************/

.section .text

.extern _kernel_phys_start
.extern _kernel_end

.extern _paged_start

.extern stack_top

.global paging_init

/*
 * This is the very first thing that the bootloader calls for our kernel.
 */

.global _start
.type _start, @function
_start:
    /* Get some CPU flags into a known state. */
    cli /* Ensure disabled interrupts */
    cld /* Clear direction flag so we can use stosX to setup the boot pages */

    /* Setup temporary stack (using physical address) */
    movl $VIRT_2_PHYS(stack_top), %esp
    movl $VIRT_2_PHYS(stack_top), %ebp
    
    /* Preserve EAX and EBX which hold boot info from multiboot */
    pushl %ebx
    pushl %eax
    
    /* Setup directory and page tables */

    /* Clear identity page. */
    movl $VIRT_2_PHYS(boot_page_identity), %edi
    movl %edi, %edx
    movl $1024, %ecx
    shrl $4, %ecx /* Storing DWORDS */

    xorl %eax, %eax
    rep stosl /* stosd */

    /* Setup kernel pages */
    movl $VIRT_2_PHYS(boot_page_kernel), %edi
    movl $1024, %ecx
    movl $0, %esi

    /* Loop start */
1:
    /* Reached end of kernel? */
    cmpl $VIRT_2_PHYS(_kernel_end), %esi
    jge 4f

    cmpl $_kernel_phys_start, %esi
    jge 2f

    /* Haven't reached the kernel yet; make the entry blank. */
    xorl %eax, %eax
    jmp 3f

2:
    /* Map the kernel page. */
    movl %esi, %eax
    orl  $0x03, %eax

3:
    /* Set entry */
    movl %eax, (%edi)
    movl %eax, (%edx)

    /* Loop tail */
    addl $0x1000, %esi
    addl $4, %edi
    addl $4, %edx
    loop 1b

4:
    /* Loop complete, blank the remaining pages and our directory */
    addl $1024, %ecx /* Include directory in blank */
    shrl $4, %ecx /* Storing DWORDS */

    xorl %eax, %eax
    rep  stosl /* stosd */

    /* Temporary debugging, add text video memory map */
    movl $0x000B8003, VIRT_2_PHYS(boot_page_identity) + VIDEO_PAGE_OFFSET

    movl $VIRT_2_PHYS(boot_page_identity), %eax
    orl  $0x03, %eax

    /* Add identity map */
    movl %eax, VIRT_2_PHYS(boot_page_directory)

    /* Add kernel virtual map */
    movl $VIRT_2_PHYS(boot_page_kernel), %eax
    orl  $0x03, %eax
    movl %eax, VIRT_2_PHYS(boot_page_directory) + KERNEL_PAGE_OFFSET

    /* Setup directory address in CR3 */
    movl $VIRT_2_PHYS(boot_page_directory), %ecx
    movl %ecx, %cr3

    /* Restore the EAX and EBX registers */
    popl %eax
    popl %ebx

    /* Turn on paging and write protection */
paging_init:
    movl %cr0, %ecx
    orl  $0x80010000, %ecx
    movl %ecx, %cr0

    /* Long jump into our _paged_start function in boot.S */
    lea _paged_start, %ecx
    jmp *%ecx

/********************************************************************************************************************/
