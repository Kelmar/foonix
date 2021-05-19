/*************************************************************************/
/*
 * $Id: loader.S 187 2011-08-27 16:27:33Z kfiresun $ 
 */
/*************************************************************************/
 
#include "asm.h"
#include "cpuflags.h"

.global loader	/* Making entry point visible to linker */

/* Setting up the multiboot header - see GRUB dogs for details. */
.set ALIGN,    1<<0             /* Align laoded modules on page boundaries */
.set MEMINFO,  1<<1             /* Provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* This is the multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find header */
.set CHECKSUM, -(MAGIC + FLAGS) /* Checksum required */

.text
.align 4
MultiBootHeader:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

/* Error strings */
NoMultibootError:
    .asciz "Not multiboot loader."

/* Reserve initial kernel stack space. */
.set STACKSIZE,	0x4000		/* This is 16k */
.comm stack, STACKSIZE, 32	/* Reserve 16k stack on quadword boundary. */

/* Reserve initial kernel page directory */
.set PD_SIZE, 0x1000		/* This is 4k */
.comm pagedir, PD_SIZE, 32	/* Reserve 4k page directory on quadword boundary. */

.set KERN_HIGH_SPOT, 0xC0010000

/*************************************************************************/
/*
 * Kernel entry point
 */
loader:
    /* Setup a temporary stack for the loader */
    mov  $(stack + STACKSIZE), %esp

    /* Write captial A so we know we're in the start up. */
    movl 0x000B800, %ecx
    movb $65, (%ecx)

    cmp 0x2BADB002, %eax
    jmp multiboot

    /* Crap!  The boot loader isn't a multiboot loader. */
    movw NoMultibootError, %si
    sti
    cld
    call print

    /* Lock the machine after printing the error. */
    cli
    hlt
.L_mb_lock:
    pause
    jmp .L_mb_lock;

multiboot:
    /* Setup some intial paging */

    /*
     * Kernel will physically placed at 0x00100000, but paged to 0xC0100000.
     * Stack starts at 0xC00FFFFF and grows down (pointer values decrease).
     *
     * To set this up we must enable paging early on with some initial
     * defaults.  The C code will have to know where the page directory is so
     * it can continue its initialization properly.
     *
     * Problem, we need to know how big the kernel currently is in order to
     * properly allocate enough page tables for it ahead of time.  The "end"
     * symbol, which is defined by the linker script, should assist with this.
     */

    /* Get size of the kernel, in 4Kbyte pages */

    /* Get the page aligned ending of the kernel */
    movl $(end), %eax
    call align_eax

    /* We assume that loader is already aligned */
    subl $(loader), %eax
    shrl $(12), %eax

    /* We'll need 256 pages to preserve the lower 1MB */
    addl $256, %eax

    /*
     * Now we need to figure out how many page tables we'll
     * need for the tables themselves.
     */
    mov %eax, %edx
    shrl $(10), %edx

    /* Ensure at least one page table gets allocated. */
    incl %edx

    /* What's the total size? */
    addl %edx, %eax

    /* EDX now has # of page tables, and EAX has total size */
    call gen_page_tables
    call gen_page_dirs

    // Each page
    mov $(end), %edx

    /* Make sure we're page aligned */
    andl $(PAGE_ALIGN_MASK), %edx
    jz .L_pt_alligned

    addl $(PAGE_SIZE), %edx

.L_pt_alligned:
    /* EDX should now point to where we want to place our page tables. */

    /* Push parameters to kmain() */
    push %ebx /* Multiboot data structure. */

    /* Write a capital B out so we'll know we're calling static ctors. */
    movb $66, (%ecx)

    /* Call static constructors */
    mov $start_ctors, %ebx
    jmp 2f
1:
    call *(%ebx)
    add $4, %ebx
2:
    cmp $end_ctors, %ebx
    jb 1b

    /* Write a capital C out so we'll know we're calling kmain(). */
    movb $67, (%ecx)

    /* Call the kernel proper */
    call kmain

    /* Call static destructors */
    mov $start_dtors, %ebx
    jmp 4f
3:
    call *(%ebx)
    add $4, %ebx
4:
    cmp $end_dtors, %ebx
    jb 3b

    /* Halt machine should kernel return. */
    cli
    hlt
.L_lock:
    pause
    jmp .L_lock

/*************************************************************************/
/**
 * Aligns EAX to the next page boundary
 */
align_eax:
    push %edx

    movl %eax, %edx
    andl $(PAGE_ALIGN_MASK), %edx

    jz .L_eax_is_aligned

    /* Align to the next page boundary */
    addl $(PAGE_SIZE), %eax
    subl %edx, %eax

.L_eax_is_aligned:

    pop %edx
    ret

/*************************************************************************/
/**
 * Generates the page tables we need to remap the kernel.
 *
 * EAX should contain the total size of the kernel with
 *   page tables in 4KByte pages.
 * EDX should contain the number of page tables needed.
 */
gen_page_tables:
    pusha

    /* Save number of PTs for kernel & PTs themselves into EDX */
    movl %eax, %edx
    subl $(256), %edx

    /* Blank out the page tables starting out */
    movl %eax, %ecx
    xorl %eax, %eax
    shrl $(2), %ecx

    rep stosl

    /* Setup location for storing the PTs */
    movl $(end), %eax
    call align_eax

    movl %eax, %edi

    /* Generate pages for lower 1MB first */
    movl $(256), %ecx

    /* Start at zero physical address */
    xorl %esi, %esi

    call gen_page_table_run

    /* Now we write out the kernel and PTs */
    movl %edx, %ecx

    /* Place physical addr into ESI */
    movl $(loader), %esi

    call gen_page_table_run

    popa
    ret

/*************************************************************************/
/**
 * Generates as set of page tables in a loop.  Starting with the
 * address in ESI and continuing for a RUN of memory in ECX size
 * of pages.
 *
 * The location of the page tables are stored at the location
 * pointed to by EDI.
 *
 * Registers destroied:
 *   EAX    (Undefined)
 *   ECX    (zero)
 *   EDI    (incremented)
 *   ESI    (Start of next frame)
 *   EFLAGS (Undefined)
 */
gen_page_table_run:
    /* Physical addr / 4096 */
    shrl $(12), %esi
    cld

.L_gen_ptrs:
    movl %esi, %eax
    shll $(12), %eax /* Should cause lower 12 bits to get lopped off */
    andl $(3), %eax  /* Present & writable page */

    stosl /* Store the PTE */

    incl %esi /* Increment our address */
    decl %ecx /* Decrement the number of entries to store */

    jnz .L_gen_ptrs

    /* Frame # * 4096 == Physical addr */
    shll $(12), %esi

    ret

/*************************************************************************/
/**
 * Generates the page directory that maps the lower 1MB and kernel's pages
 */
gen_page_dirs:
    pusha

    /* Make sure page directory is nice and clean */
    movl $(PD_SIZE), %ecx
    shrl $2, %ecx

    movl $(pagedir), %edi

    xorl %eax, %eax

    cld
    rep stosl

    /*
     * Our page tables are assumed to be at the next page boundary
     * at or after the "end" symbol.
     */
    movl $(end), %eax
    call align_eax

    mov %eax, %esi

    popa
    ret

/*************************************************************************/
/**
 * Prints a message pointed to in DS:ESI
 *
 * Currently requires real mode
 */
print:
    pusha

.L_next_char:
    lodsb
    orb %al,%al
    jz .exit_print
    movb 0x0E, %ah
    movw $7, %bx
    int $0x10
    jmp .L_next_char

.exit_print:
    popa
    ret
