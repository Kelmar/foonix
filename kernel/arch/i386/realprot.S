#include "cpuflags.h"

gdt:
	.word 0, 0
	.byte 0, 0x00, 0x00, 0

.globl idt_real
idt_real:
	.word 0x03ff	# 256 entries, 4b each = 1K
	.word 0		# Real mode IVT @ 0x0000

.global real_to_prot;
real_to_prot:
	.code16
	cli			# Disable interrupts
	lgdt gdt		# Load GDT
	mov  %cr0, %eax		# Set PE bit
	or   $CR0_PE, %eax
	mov  %eax, %cr0
	ljmp $0, $1f
1:
	.code32
	ret			# Return to caller

.global prot_to_real;
prot_to_real:
	.code32
# Disable paging
	mov  %cr0, %eax		# Unset PE bit
	and  $~CR0_PE, %eax
	mov  %eax, %cr0
	ljmp $0, $1f
1:
	.code16
	mov  0x8000, %sp	# Magic number stack pointer....
	xor  %ax, %ax
	mov  %ax, %ds
	mov  %ax, %es
	mov  %ax, %fs
	mov  %ax, %gs
	lidt idt_real		# Load IDT
	sti			# Enable interrupts
	ret
