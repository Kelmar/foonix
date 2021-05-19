#include "asm.h"
#include "cpuflags.h"

_LABEL(cli)
	cli
	ret
	
_LABEL(sti)
	sti
	ret

_LABEL(outb)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outb  %al, %dx
	ret

_LABEL(inb)
	movl  4(%esp), %edx
	xorl  %eax, %eax
	inb   %dx, %al
	ret

_LABEL(outw)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outw  %ax, %dx
	ret

_LABEL(inw)
	movl  4(%esp), %edx
	xorl  %eax, %eax
	inw   %dx, %ax
	ret

_LABEL(outl)
	movl  4(%esp), %edx
	movl  8(%esp), %eax
	outl  %eax, %dx
	ret

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

_LABEL(khalt)
	cli
	hlt
.L_hlt:
	pause
	jmp .L_hlt

/*
 * Load a global descriptor table:
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
	
_LABEL(set_page)
	movl 4(%esp), %eax
	movl %eax, %cr3

	/* Enable paging and flush page cache. */
	movl %cr0, %eax
	orl  $CR0_PG, %eax
	movl %eax, %cr0
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
