/********************************************************************************************************************/
/*
 * $Id: cpuflags.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/********************************************************************************************************************/

#ifndef __FOONIX_CPU_FLAGS_H__
#define __FOONIX_CPU_FLAGS_H__

/********************************************************************************************************************/
/*
 * List of 80x86 CPU flags.
 */

#define CR0_PE 0x00000001 /* Protected Mode Enable */
#define CR0_MP 0x00000002 /* Monitor co-processor */
#define CR0_EM 0x00000004 /* Emulation */
#define CR0_TS 0x00000008 /* Task switched */
#define CR0_ET 0x00000010 /* Extention type */
#define CR0_NE 0x00000020 /* Numeric error */
#define CR0_WP 0x00010000 /* Write protect */
#define CR0_AM 0x00040000 /* Alignment mask */
#define CR0_NW 0x20000000 /* Not-write through */
#define CR0_CD 0x40000000 /* Cache disable */
#define CR0_PG 0x80000000 /* Paging */

#define CR4_VME 0x00000001 /* Virtual 8086 mode extentions */
#define CR4_PVI 0x00000002 /* Protected Mode Virtual Interrupts */
#define CR4_TSD 0x00000004 /* Time Stamp Disable */
#define CR4_DE  0x00000008 /* Debugging Extensions */
#define CR4_PSE 0x00000010 /* Page Size Extensions */
#define CR4_PAE 0x00000020 /* Physical Address Extension */
#define CR4_MCE 0x00000040 /* Machine Check Exception */
#define CR4_PGE 0x00000080 /* Page Global Enabled */
#define CR4_PCE 0x00000100 /* Performance-Monitoring Counter enable */

/* Operationg system support for FXSAVE and FXSTOR inst */
#define CR4_OSFXSR 0x00000200

/* OS support for unmaskes SIMD FP exceptions */
#define CR4_OSXMMEXCPT 0x00000400

#define CR4_VMXE 0x00002000	/* VMX Enable */

/********************************************************************************************************************/

#endif /* __FOONIX_CPU_FLAGS_H__ */

/********************************************************************************************************************/
