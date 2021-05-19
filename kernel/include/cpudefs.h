/*************************************************************************/
/*
 * $Id: cpudefs.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/

#ifndef __FOO_CPUDEFS_H__
#define __FOO_CPUDEFS_H__

/*************************************************************************/

#define IRQ_START 32
#define IRQISR(X) (X + IRQ_START)

#define PAGE_SIZE 4096 /* in bytes */
#define PAGE_ALIGN_MASK ((~PAGE_SIZE) + 1)

/*************************************************************************/

#endif /* __FOO_CPUDEFS_H__ */

/*************************************************************************/