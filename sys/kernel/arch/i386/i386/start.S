/*************************************************************************/
/*
 * This is our initial boot loader after stage 0 or 1.  We get called from 
 * evertyhign here, EFI, multiboot and so on.
 *
 * This file establishes a few conventions to make sure we preseve some information:
 *
 * 1) We need to preserve the initial contents of EAX, which for multiboot has the
 *    multiboot header that we'll want to parse in the C code.
 */
/*************************************************************************/

#include "asm.h"

# Setting up the multiboot header - see GRUB docs for details.
.set ALIGN,     1<<0                # Align laoded modules on page boundaries
.set MEMINFO,   1<<1                # Provide memory map
.set FLAGS,     ALIGN | MEMINFO     # This is the multiboot 'flag' field
.set MAGIC,     0x1BADB002          # 'magic number' lets bootloader find header
.set CHECKSUM,  -(MAGIC + FLAGS)    # Checksum required

.set TEXTADDR,  0x000B8000          # Physical location of VGA text mode
.set HIGHADDR,  0xC0000000

/*************************************************************************/

# Multiboot header
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*************************************************************************/

.section .bss, "aw", @nobits
    /* Boot pages */
    .align 4096
    LABEL(boot_page_identity)
    .skip 4096

    .align 4096
    LABEL(boot_page_directory)
    .skip 4096

/*************************************************************************/

.section .multiboot.text, "a"

.extern _kernel_phys_start
.extern _kernel_end

.extern _paged_start

.global _start
.type _start, @function
_start:
    /* Make sure interrupts are disabled for our startup. */
    cli

    /* Preserve EAX in EDX, we need this for multiboot detection. */
    movl %eax, %edx

    /* Setup directory and page tables */
    movl $VIRT_2_PHYS(boot_page_identity), %edi
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

    /* Loop tail */
    addl $0x1000, %esi
    addl $4, %edi
    loop 1b

4:
    /* Loop complete, blank the remaining pages and our directory */
    addl $1024, %ecx /* Includ directory in blank */
    xorl %eax, %eax
    rep  stosl /* stosd */

    /* Tempoary debugging, add text video memory map at 0xC03FF000 */
    movl $0x000B8003, VIRT_2_PHYS(boot_page_identity) + (1023 * 4)

    movl $VIRT_2_PHYS(boot_page_identity), %eax
    orl  $0x03, %eax

    /* Add identity map and virtual map */
    movl %eax, VIRT_2_PHYS(boot_page_directory) /* Identity map */
    movl %eax, VIRT_2_PHYS(boot_page_directory) + (768 * 4) /* Virtual map */

    /* Setup directory address in CR3 */
    movl $VIRT_2_PHYS(boot_page_directory), %ecx
    movl %ecx, %cr3

    /* Turn on paging and write protection */
    movl %cr0, %ecx
    orl  $0x80010000, %ecx
    movl %ecx, %cr0

    /* Long jump into our _paged_start function */
    lea _paged_start, %ecx
    jmp *%ecx

/*************************************************************************/
