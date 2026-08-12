/********************************************************************************************************************/
/*
 * Initial entry point for our kernel.
 *
 * Checks for x64 support and moves us into long mode if found.
 *
 * This file establishes a few conventions to make sure we preserve some information:
 *
 * 1) We need to preserve the initial contents of EAX and EBX, which for multiboot has the multiboot information we
 *    need later in the C++ code.
 */
/********************************************************************************************************************/

#include "asm.h"

/********************************************************************************************************************/

#define CPUID_FLAG (1 << 21)

#define CPUID_MAX_LEAF 0x80000000
#define CPUID_FEATURES 0x80000001

#define CPUID_FEATURE_X64 (1 << 29)

#define VGA_MEMORY 0x000B8000

/********************************************************************************************************************/

.section .bss, "aw", @nobits
    /* Boot pages */
    .align 4096
    LABEL(boot_pml4t)
    .skip 4096

    LABEL(boot_pdpt)
    .skip 4096

    LABEL(boot_pdt)
    .skip 4096

    LABEL(boot_pt)
    .skip 4096

/********************************************************************************************************************/

.section .data
    /* System x32 global descriptor table */
    .align 8
LABEL(g_gdtEntries)
    .quad 0x0000000000000000 /* NULL descriptor */
    .quad 0x00209A0000000000 /* Code segment (Kernel) */
    .quad 0x0020920000000000 /* Data segment (Kernel) */
    .quad 0x0020FA0000000000 /* Code segment (User) */
    .quad 0x0020F20000000000 /* Data segment (User) */
    .quad 0x0000000000000000 /* TSS */

    .align 4
LABEL(g_gdtPtr)
    .word . - g_gdtEntries - 1
    .long g_gdtEntries

/********************************************************************************************************************/

.section .text

.code32

.extern _kernel_phys_start
.extern _kernel_end

.extern _x64_start

.extern stack_top

.global _start
.type _start, @function
_start:
    /* Get some CPU flags in a known state. */
    cli /* Ensure interrupts are disabled */
    cld /* Clear direction flag for stosX instructions */

    /* Setup temporary stack (using physical address) */
    movl $stack_top, %esp
    movl $stack_top, %ebp

    /* Preserve EAX and EBX which may hold boot info from multiboot */
    pushl %ebx
    pushl %eax

    /* Check to see if we have CPUID available. */
    pushfd
    popl %eax
    movl %eax, %ecx

    xorl $CPUID_FLAG, %eax
    pushl %eax
    popfd
    pushfd
    popl %eax

    xorl %ecx, %eax
    jz 1f

    /* CPUID found, see what the maximum leaf we can get. */
    movl $CPUID_MAX_LEAF, %eax
    cpuid

    /* Check to see if it at least supports CPUID_FEATURES */
    cmpl $CPUID_FEATURES, %eax
    jb 1f

    /* Okay, NOW we actually check for x64 support. */
    movl $CPUID_FEATURES, %eax
    cpuid
    testl $CPUID_FEATURE_X64, %edx
    jz 1f

    /* Congrats!  It looks like x64 is supported, let's get the page tables setup. */

    /* Setup a minimal long mode page table for our kernel to boot up with. */
    movl $boot_pdpt, %eax
    orl $0x3, %eax // Double check flags
    movl %eax, boot_pml4t

    movl $boot_pdt, %eax
    orl $0x3, %eax // Double check flags
    movl %eax, boot_pdpt

    movl $boot_pt, %eax
    movl %eax, %edi
    orl $0x3, %eax // Double check flags
    movl %eax, boot_pdt

    /* Identity map first 2MiB of addresses */
    movl $0x03, %ebx // Mark page as present and readable.
    movl $512, %ecx  // Number of entries.

1:
    movl %ebx, (%edi)
    addl $0x1000, %ebx // Page size
    addl $8, %edi      // Size of page table entry
    loop 1b

    popl %eax
    popl %ebx

    movl $boot_pml4t, %edi
    movl %edi, %cr3 // Load up our new page table addresses.

    // Enable PAE
    movl %cr4, %ecx
    orl $(1 << 5), %ecx
    movl %ecx, %cr4

    /*
     * Using EDI to preserve EAX while we transition to x64 mode w/o using the stack.
     *
     * We can't use EBX because that has it's own value we need to preserve.
     * rdmsr/wrmsr use EAX, ECX, and EDX....
     */
    movl %eax, %edi

    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    movl %cr0, %eax
    orl $(1 << 31), %eax
    movl %eax, %cr0

    /* Restore EAX */
    movl %edi, %eax

    /* Load the global descriptor table */
    lgdt g_gdtPtr

    ljmp $(1 << 3), $_x64_start /* Set long mode and CS value */

1:  /* CPU Not Supported */
    movl $VGA_MEMORY, %edi
    movl $cpu_not_supported, %esi

2: /* Write message to VGA memory */
    lodsb
    testb %al, %al
    jz 3f

    stosb

    /* Write color data */
    movb $0x07, %al
    stosb

    jmp 2b

3: /* Not supported death loop */
    pause
    hlt
    jmp 3b

/********************************************************************************************************************/

.section .rodata

cpu_not_supported: .asciz "CPU Not Supported"

/********************************************************************************************************************/
