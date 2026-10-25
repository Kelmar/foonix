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
#include "cpudefs.h"

/********************************************************************************************************************/

#define PF_PRESENT  0x001
#define PF_WRITABLE 0x002
#define PF_LARGE    0x080

/********************************************************************************************************************/

#define CPUID_FLAG (1 << 21)

#define CPUID_MAX_LEAF 0x80000000
#define CPUID_FEATURES 0x80000001

#define CPUID_FEATURE_1GB_PG (1 << 26)
#define CPUID_FEATURE_X64 (1 << 29)

#define STEP_2MB 0x00200000

#define CR0_PAGING (1 << 31)

#define CR4_PAE (1 << 5)

#define ENTRY_SIZE 8
#define ENTRY_COUNT 512

#define LAST_ENTRY (ENTRY_SIZE * (ENTRY_COUNT - 1))

#define VGA_MEMORY 0x000B8000

/********************************************************************************************************************/

.section .bss, "aw", @nobits

# Reserve L4 page table
    .align 4096
LABEL(g_kernelTable)
    .skip 4096

# Reserve initial kernel stack space.
    .align 16
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

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
    .long VIRT_2_PHYS(g_gdtEntries)

/********************************************************************************************************************/

.section .text

.code32

.extern _kernel_phys_start
.extern _bss_start
.extern _kernel_end

.extern _x64_start

.extern g_bootAllocatedPages

.global _start
.type _start, @function
_start:
    /* Get some CPU flags in a known state. */
    cli /* Ensure interrupts are disabled */
    cld /* Clear direction flag for stosX instructions */

    /* Setup temporary stack (using physical address) */
    movl $VIRT_2_PHYS(stack_top), %esp
    movl $VIRT_2_PHYS(stack_top), %ebp

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
    jz .no_x64

    /* CPUID found, see what the maximum leaf we can get. */
    movl $CPUID_MAX_LEAF, %eax
    cpuid

    /* Check to see if it at least supports CPUID_FEATURES */
    cmpl $CPUID_FEATURES, %eax
    jb .no_x64

    /* Okay, NOW we actually check for x64 support. */
    movl $CPUID_FEATURES, %eax
    cpuid
    testl $CPUID_FEATURE_X64, %edx
    jnz .has_x64

.no_x64:  /* CPU Not Supported */
    movl $VGA_MEMORY, %edi
    movl $VIRT_2_PHYS(cpu_not_supported), %esi

.write_loop: /* Write message to VGA memory */
    lodsb
    testb %al, %al
    jz .death_loop

    stosb

    /* Write color data */
    movb $0x07, %al
    stosb

    jmp .write_loop

.death_loop: /* Not supported death loop */
    pause
    hlt
    jmp .death_loop

.has_x64:
    // Congrats!  It looks like x64 is supported, let's get the page tables setup.

    // Our temp stack is in the BSS section, so restore our values for a moment.
    popl %ebp // Saving eax in ebp for this process
    popl %ebx

    // Ensure our BSS section is blanked out.
    movl $VIRT_2_PHYS(_bss_start), %edx
    movl %edx, %edi
    movl $VIRT_2_PHYS(_kernel_end), %ecx
    subl %edx, %ecx
    movl %ecx, %edx
    shrl $2, %ecx

    xorl %eax, %eax
    rep stosl

    movl %edx, %ecx
    andl $3, %ecx
    rep stosb

    // Okay to push values back into stack
    pushl %ebx
    pushl %ebp
    
    // Build up a set of boot up pages after the kernel which we will pass on to the boot args structure later.
    // These pages use large page allocations to keep things as minimal here in assembly as possible.

    // Get the start of our heap and start allocating pages for our tables.
    // edx to hold the start of our heap through out this process.
    .global .kern_pg_begin
.kern_pg_begin:

    // Align to page boundary just after the kernel's physical memory.
    movl $VIRT_2_PHYS(_kernel_end), %edx
    addl $(PAGE_SIZE - 1), %edx
    andl $(~(PAGE_SIZE - 1)), %edx

    // edi is also our heap pointer.
    movl %edx, %edi

    // Preserve location of L3 tables
    movl %edx, %ebx

    // Blank two L3 pages
    movl $(PAGE_SIZE >> 1), %ecx
    xorl %eax, %eax
    rep stosl

    // Add L3 entry to L4 table.
    movl %ebx, %eax
    orl $(PF_PRESENT | PF_WRITABLE), %eax
    
    // Write to first entry in L4 table
    movl %eax, VIRT_2_PHYS(g_kernelTable)
    
    // Write second table to last entry in L4 table
    addl $PAGE_SIZE, %eax
    movl %eax, VIRT_2_PHYS(g_kernelTable + LAST_ENTRY)

    // edi should be pointing at where we can allocate some L2 entries.

    // Setup L2 entries in our L3 tables
    movl %edi, %eax
    orl $(PF_PRESENT | PF_WRITABLE), %eax

    movl %ebx, %ecx

    // For second L3 table, entries go at the end.
    addl $(PAGE_SIZE + LAST_ENTRY - ENTRY_SIZE), %ecx

    movl %eax, (%ebx)
    movl %eax, (%ecx)

    addl $PAGE_SIZE, %eax
    addl $ENTRY_SIZE, %ebx
    addl $ENTRY_SIZE, %ecx

    movl %eax, (%ebx)
    movl %eax, (%ecx)

    // Now fill in the 2MB entries in our L2 tables, generating an identity map to first 2GB of memory, using 2MB pages.
    movl $(PF_LARGE | PF_PRESENT | PF_WRITABLE), %eax // Setup for 2MB pages
    xorl %ebx, %ebx
    movl $(PAGE_SIZE >> 2), %ecx

.l2_fill:
    // Write single entry
    movl %eax, (%edi)
    addl $4, %edi
    movl %ebx, (%edi)
    addl $4, %edi

    addl $STEP_2MB, %eax
    loop .l2_fill

    // Now figure out how many pages we've allocated and inform the preinit so it doesn't clobber our info.
    movl %edi, %eax
    subl %edx, %eax
    shrl $PAGE_SHIFT, %eax
    movl %eax, VIRT_2_PHYS(g_bootAllocatedPages)

    .global .kern_pg_setup
.kern_pg_setup:
    /*
     * Restore multiboot values into edi and esi for call to preinit.
     *
     * Somewhat conveniently we don't need edi or esi for the below instructions
     * but we do need eax, ebx?, ecx, and edx.  However, Clang is using the SysV ABI
     * which passes the first and second parameters in edi and esi respectively.
     */
    popl %edi
    popl %esi

    movl $VIRT_2_PHYS(g_kernelTable), %eax
    movl %eax, %cr3 // Load up our new page table addresses.

    // Enable PAE
    movl %cr4, %ecx
    orl $CR4_PAE, %ecx
    movl %ecx, %cr4

    // TODO: Figure out what this magic is doing.....
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    // Enable paging
    movl %cr0, %eax
    orl $CR0_PAGING, %eax
    movl %eax, %cr0

    // Load the global descriptor table
    lgdt VIRT_2_PHYS(g_gdtPtr)

    // Jump to long mode trampoline with a 32-bit address.
    ljmp $(1 << 3), $VIRT_2_PHYS(_lm_trampoline)

/********************************************************************************************************************/

.code64

.global _lm_trampoline
.type _lm_trampoline, @function
_lm_trampoline:
    // Jump from 32-bit address space to 64-bit now that we're in 64-bit mode.
    movabsq $_longmode, %rax
    jmp *%rax // Boing!

.global _longmode
.type _longmode, @function
_longmode:
    // Need to adjust segment registers before we can use memory.
    xorw %dx, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %fs
    movw %dx, %gs
    movw %dx, %ss

    // Setup temporary stack (using virtual addresses)
    movq $stack_top, %rsp
    movq $stack_top, %rbp

    // Note that rdi and rsi were loaded from _start prior to the long mode jump.
    // This has our multiboot parameters in them.

    call preinit // Initialize CPU & VM first

    movq %rax, %rdi

    call kmain

    cli // Ensure interrupts are disabled if we get here.

1:
    // Halt machine.
    hlt
    pause
    jmp 1b

.section .rodata

cpu_not_supported: .asciz "CPU Not Supported"

/********************************************************************************************************************/
