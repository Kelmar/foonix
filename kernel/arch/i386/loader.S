#include "asm.h"
#include "cpuflags.h"

.global loader	/* Making entry point visible to linker */

/* Setting up the multiboot header - see GRUB dogs for details. */
.set ALIGN,	1<<0		 /* Align laoded modules on page boundaries */
.set MEMINFO,	1<<1		 /* Provide memory map */
.set FLAGS,	ALIGN | MEMINFO	 /* This is the multiboot 'flag' field */
.set MAGIC,	0x1BADB002	 /* 'magic number' lets bootloader find header */
.set CHECKSUM,	-(MAGIC + FLAGS) /* Checksum required */

.text
.align 4
MultiBootHeader:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

/* Reserve initial kernel stack space. */
.set STACKSIZE,	0x4000		/* This is 16k */
.comm stack, STACKSIZE, 32	/* Reserve 16k stack on quadword boundary. */

loader:
    /* Write captial A so we know we're in the start up. */
    movl 0x000B800, %ecx
    movb $65, (%ecx)

    /* Setup the stack */
    mov  $(stack + STACKSIZE), %esp
    
    push %eax /* Multiboot magic number */
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
    
