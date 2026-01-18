#include "../asm.h"

/* void memset(void *dst, uint8_t byte, uint32_t cnt); */
_LABEL(memset)
	movzbl 8(%esp), %eax
	movl  12(%esp), %ecx

	pushl %edi
	movl 8(%esp), %edi

	/* Copy char to whole EAX register. */
	movb %al, %ah
	movl %eax, %edx
	sall $16, %eax
	orl  %edx, %eax
	cld

	/* Store by dwords. */
	shrl $2, %ecx
	rep stosl

	/* Check for left over bytes. */
	movl 16(%esp), %ecx
	andl $3, %ecx
	jz .L_exit

	rep stosb

.L_exit:
	movl 8(%esp), %eax
	popl %edi
	ret
