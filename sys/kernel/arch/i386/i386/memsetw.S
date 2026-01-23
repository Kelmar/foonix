/********************************************************************************************************************/
/*
 * Copyright (c) 2009 Bryce Simonds
 *
 * A 16bit word fill for foonix
 */
/********************************************************************************************************************/

#include "asm.h"

/********************************************************************************************************************/

/* void *_memsetw(void *dst, uint16_t word, uint32_t cnt); */
_LABEL(_memsetw)
	xor %eax, %eax
	movw 8(%esp), %ax
	movl 12(%esp), %ecx

	pushl %edi
	movl 8(%esp), %edi

	/* Copy word to whole EAX register. */
	movl %eax, %edx
	sall $16, %eax
	orl  %edx, %eax
	cld

	/* Store by dwords. */
	shrl $1, %ecx
	rep stosl

	/* Check for left over words. */
	movl 16(%esp), %ecx
	andl $1, %ecx
	jz .L_exit

	stosw
	
.L_exit:
	movl 8(%esp),%eax
	popl %edi
	ret

/********************************************************************************************************************/
