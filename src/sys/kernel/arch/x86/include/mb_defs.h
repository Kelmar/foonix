/********************************************************************************************************************/
/*
 * Multiboot 1 definitions.
 *
 * Used by C++ and ASM code.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MB_DEFS_H__
#define __FOONIX_KERNEL_MB_DEFS_H__

/********************************************************************************************************************/
// Multiboot header values.

#define MB_MAGIC 0x1BADB002

#define MB_REQ_ALIGN   (1 << 0)
#define MB_REQ_MEMINFO (1 << 1)
#define MB_REQ_VIDEO   (1 << 2)

/********************************************************************************************************************/

#define MULTIBOOT_MAGIC 0x2BADB002

#define MB_FLAG_MEM 0x00000001
#define MB_FLAG_BOOTDEV 0x00000002

#define MB_BOOT_DRIVE 0
#define MB_BOOT_PART1 1 /* Primary partition (includes DOS ext, starting at 4)*/
#define MB_BOOT_PART2 2 /* "Sub" partition, (BSD partitions) */
#define MB_BOOT_PART3 3 /* Sub-Sub partition.... */

#define MB_FLAG_CMDLINE 0x00000004
#define MB_FLAG_MODS 0x00000008
#define MB_FLAG_AOUTSYMS 0x00000010
#define MB_FLAG_ELFSYMS 0x00000020
#define MB_FLAG_MMAP 0x00000040
#define MB_FLAG_DRIVERS 0x00000080
#define MB_FLAG_CONFIG 0x00000100
#define MB_FLAG_BLNAME	0x00000200
#define MB_FLAG_APM	0x00000400
#define MB_FLAG_VBE	0x00000800

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MULTBOOT_H__ */

/********************************************************************************************************************/
