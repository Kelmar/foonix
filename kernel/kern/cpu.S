/*************************************************************************/
/*
 * $Id: cpu.S 98 2010-10-18 07:21:17Z kfiresun $ 
 */
 /*************************************************************************/
 
#include "asm.h"
#include "cpuflags.h"

/* void cli(void); */
_LABEL(cli)
	cli
	ret
	
/* void sti(void); */
_LABEL(sti)
	sti
	ret

/* void outb(int port, int byte); */
_LABEL(outb)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outb  %al, %dx
	ret

/* int inb(int port); */
_LABEL(inb)
	movl  4(%esp), %edx
	xorl  %eax, %eax
	inb   %dx, %al
	ret

/* void outw(int port, int word); */
_LABEL(outw)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outw  %ax, %dx
	ret

/* int inw(int port); */
_LABEL(inw)
	movl  4(%esp), %edx
	xorl  %eax, %eax
	inw   %dx, %ax
	ret

/* void outl(int port, int dword); */
_LABEL(outl)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outl  %eax, %dx
	ret
	
/* int inl(int port); */
_LABEL(inl)
	movl  4(%esp), %edx
	inl   %dx, %eax
	ret
	
_LABEL(insb)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	cld
	rep insb
	pop %edi
	leave
	ret

_LABEL(outsb)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	cld
	rep outsb
	pop %edi
	leave
	ret
	
_LABEL(insw)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	shrl $1, %ecx
	cld
	rep insw
	pop %edi
	leave
	ret
	
_LABEL(outsw)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	shrl $1, %ecx
	cld
	rep outsw
	pop %edi
	leave
	ret
	
_LABEL(insl)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	shrl $2, %ecx
	cld
	rep insl
	pop %edi
	leave
	ret
	
_LABEL(outsl)
	push %ebp
	movl %esp, %ebp
	push %edi
	movl 8(%ebp), %edx
	movl 12(%ebp), %edi
	movl 16(%ebp), %ecx
	shrl $2, %ecx
	cld
	rep outsl
	pop %edi
	leave
	ret

/*
 * Halts execution of the system.
 *
 * void khalt(void);
 */
_LABEL(khalt)
	cli
	hlt
.L_hlt:
	pause
	jmp .L_hlt

/*
 * Load a global descriptor table.
 *
 * void load_gdt(struct gdt_ptr *)
 */
_LABEL(load_gdt)
	movl 4(%esp), %eax
	lgdt (%eax)
	jmp  1f
	nop
1:
	mov  $0x10, %ax
	mov  %ax, %ds
	mov  %ax, %es
	mov  %ax, %fs
	mov  %ax, %gs
	mov  %ax, %ss
	ret

_LABEL(process_prestub)
	call process_stub
	ret
	
/*
 * Sets the page directory.
 *
 * void set_pagedir(void *pd);
 */
_LABEL(set_pagedir)
	movl 4(%esp), %eax
	movl %eax, %cr3

	/* Enable paging and flush page cache. */
	movl %cr0, %eax
	orl  $CR0_PG, %eax
	movl %eax, %cr0
	ret

_LABEL(flush_page_cache)
	movl %cr3, %eax
	movl %eax, %cr3
	ret
	
/* Read's the value of the EAX register. */
_LABEL(read_eax)
	ret

/* Read's the value of the CR0 register. */
_LABEL(read_cr0)
	movl %cr0, %eax
	ret
	
/* Read's the value of the CR2 register. */
_LABEL(read_cr2)
	movl %cr2, %eax
	ret

/* Read's the value of the CR3 register. */
_LABEL(read_cr3)
	movl %cr3, %eax
	ret

/* Read's the value of the CR4 register. */
_LABEL(read_cr4)
	movl %cr4, %eax
	ret

_LABEL(load_cr0)
	movl 4(%esp), %eax
	movl %eax, %cr0
	ret
	
_LABEL(load_cr3)
	movl 4(%esp), %eax
	movl %eax, %cr3
	ret

_LABEL(load_cr4)
	movl 4(%esp), %eax
	movl %eax, %cr4
	ret
	
/* Reads the SS register */
_LABEL(read_ss)
	movl %ss, %eax
	ret
	
_LABEL(read_cs)
	movl %cs, %eax
	ret
	
/* Reads the EBP register */
_LABEL(read_ebp)
	movl %ebp, %eax
	ret

/* Reads the ESP regster */
_LABEL(read_esp)
	movl %esp, %eax
	ret

/* Read's the current EIP register. */
_LABEL(read_eip)
	pop %eax
	jmp %eax
	
/* Reads the EFLAGS regster */
_LABEL(read_eflags)
	pushf
	pop %eax
	ret

/*
 * Reads the CPU counter.
 *
 * uint32_t read_cpu_counter(uint32_t *lo_value, uint32_t *hi_value);
 */
_LABEL(read_cpu_counter)
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	
	rdtsc
	
	/* Check for NULL pointer */
	test %ebx, %ebx
	jz 1f
	movl %eax, (%ebx)
1:
	/* Check for NULL pointer */
	test %ecx, %ecx
	jz 1f
	movl %edx, (%ecx)
1:
	ret

/*
 * Returns the number of seconds currently in the RTC.
 * Used for timing.
 *
 * uint32_t read_rtc_second(void);
 */
_LABEL(read_rtc_second)
	xorl %eax, %eax
	outb %al, $0x40
	pause
	inb $0x41, %al
	ret
	
/*
 * Reads the current time of day from the RTC.
 *
 * Value returned is the number of seconds sense midnight.
 */
_LABEL(read_rtc_time)
	xorl %eax, %eax
	movl $60, %edx
	
	/* Read the hour */
	movb $4, %al
	outb %al, $0x40
	pause
	inb $0x41, %al
	mul %eax
	movl %eax, %ebx
	
	/* Read the minutes */
	xorl %eax, %eax
	movb $2, %al
	outb %al, $0x40
	pause
	inb $0x41, %al
	mul %eax
	addl %eax, %ebx
	
	/* Read the seconds */
	xorl %eax, %eax
	outb %al, $0x40
	pause
	inb $0x41, %al
	addl %ebx, %eax
	
	ret

/*
 * Deliberately causes a page fault by dereferencing a NULL pointer.
 *
 * Sets the registers to known values.
 */
_LABEL(force_crash)
	movl $0xAAAAAAAA, %eax
	movl $0xBBBBBBBB, %ebx
	movl $0xCCCCCCCC, %ecx
	movl $0xDDDDDDDD, %edx
	movl $0xEEEEEEEE, %edi
	movl $0xFFFFFFFF, %esi
	
	movl (%edi), %esi // Page fault now
	ret // Shouldn't get here

/*
 * Provides a brief short delay, the length of the delay is highly dependent
 * on the speed of the computer the software is running on!
 *
 * This function is for creating delays when we need to wait on a hardware
 * response.
 */
_LABEL(soft_delay)
	movl $2000, %ecx
.L_loop:
	pause
	loop .L_loop
	ret

/*
 * Reboots the PC.
 *
 * int reboot(void);
 *
 * Returns 0xFFFFFFFF on failure.
 */
_LABEL(reboot)
    /* Flush the keyboard buffer */
    cli
    
.L_flush_top:
    movl $0x64, %edx
    inb %dx, %al
    movb %bl, %al
    andb $1, %al
    jz .L_flush_test
    
    movl $0x60, %edx
    inb %dx, %al
    
.L_flush_test:
    andb $2, %bl
    jnz .L_flush_top
    
    /* Send reboot command to keyboard.... */
    movl $0x60, %edx
    movl $0xFE, %eax
    outb %al, %dx
    
    /* If we got here, the computer didn't reboot.... */
    movl $0xFFFFFFFF, %eax
    ret

/**
 * Copies a page of physical memory from one address to another.
 *
 * void copy_page(uintptr_t dest_physical, uintptr_t source_physical);
 */
_LABEL(copy_page)
    movl %edi, %edx
    pushl %esi
	
    movl  8(%esp), %edi
    movl 12(%esp), %esi
    movl $400, %ecx

    /* Preserve the state of interrupt handling. */
    pushf
    cli /* Disable interrupts (if they aren't already) */

    /* Disable paging for a minute */
    movl %cr0, %eax
    andl $0x7FFFFFFF, %eax
    movl %eax, %cr0
		
    /* Now copy the page */
    rep stosl
	
    /* Re-enable paging */
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    popf /* Restores state of interrupt handling */
	
    popl %esi
    mov %edx, %edi
    ret
