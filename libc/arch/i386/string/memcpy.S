/*************************************************************************/
/*************************************************************************/

#include "../asm.h"

/*************************************************************************/

/* void memcpy(void *dst, const void *src, uint32_t cnt); */
_LABEL(memcpy)
	movl  %edi, %edx
	pushl %esi
	movl  8(%esp), %edi
	movl  12(%esp), %esi
	movl  16(%esp), %ecx
	movl  %ecx, %eax

	/* Copy dwords */
	cld
	shrl  $2, %ecx
	rep   movsl

	/* Any bytes left? */
	and  $3, %eax
	jz   .L_exit

	/* Copy remaining bytes */
	movl  %eax, %ecx
	rep   movsb

.L_exit:
	movl  %edx, %edi
	popl  %esi
	ret

/*************************************************************************/
