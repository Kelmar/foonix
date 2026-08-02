/********************************************************************************************************************/
/*
 * Multiboot 2 definitions.
 *
 * Used by C++ and ASM code.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MB2_DEFS_H__
#define __FOONIX_KERNEL_MB2_DEFS_H__

/********************************************************************************************************************/

// Multiboot 2 constants

/** @brief Magic value for Multiboot header detection. */
#define MB2_HDR_MAGIC 0xE85250D6

/** @brief Magic value for Multiboot2 start detection. */
#define MB2_MAGIC 0x36D76289

#define MB2_ARCH_I386   0
#define MB2_ARCH_MIPS32 4

/* Header Tags */
#define MB2_HDR_TAG_END         0 /* Terminator tag */
#define MB2_HDR_TAG_INFO        1 /* Information request */
#define MB2_HDR_TAG_ADDRESS     2
#define MB2_HDR_TAG_ENTRY_ADDR  3 /* General entry point address */
#define MB2_HDR_TAG_FLAGS       4 
#define MB2_HDR_TAG_FRAMEBUFFER 5 /* Request framebuffer setup. */
#define MB2_HDR_TAG_MOD_ALIGN   6 /* Module alignment */
#define MB2_HDR_TAG_EFI_SERVICE 7 /* EFI Boot Service */
#define MB2_HDR_TAG_EFI32_ENTRY 8 /* EFI 32-bit entry point. */
#define MB2_HDR_TAG_EFI64_ENTRY 9 /* EFI 64-bit entry point. */
#define MB2_HDR_TAG_RELOCATABLE 10

/* Tag flags */
#define MB2_TF_NONE     0 /* No flags for this tag */
#define MB2_TF_OPTIONAL 1

/********************************************************************************************************************/
/* Info tags */
#define MB2_TAG_END           0 /* Terminator tag */
#define MB2_TAG_BOOT_CMD      1
#define MB2_TAG_BOOTLOADER    2
#define MB2_TAG_MODULES       3
#define MB2_TAG_BASIC_MEMINFO 4
#define MB2_TAG_BIOS_BOOT_DEV 5
#define MB2_TAG_MEMORY_MAP    6
#define MB2_TAG_VBE           7
#define MB2_TAG_FRAMEBUFFER   8
#define MB2_TAG_ELF_SYMS      9
#define MB2_TAG_APM           10
#define MB2_TAG_EFI_SYS32     11
#define MB2_TAG_EFI_SYS64     12
#define MB2_TAG_SMBIOS        13
#define MB2_TAG_ACPI_10       14 /* ACPI 1.0 */
#define MB2_TAG_ACPI_20       15 /* ACPI 2.0 */
#define MB2_TAG_NET           16
#define MB2_TAG_EFI_MEM_MAP   17
#define MB2_TAG_EFI_BOOT      18
#define MB2_TAG_EFI_IMG32     19
#define MB2_TAG_EFI_IMG64     20
#define MB2_TAG_BASELOAD_PTR  21


#define MB2_MEMTYPE_AVAILABLE 1     // Regular normal available RAM
#define MB2_MEMTYPE_ACPI      3     // Usable ACPI memory
#define MB2_MEMTYPE_HIBERNATE 4     // Memory to preserve on hibernate.
#define MB2_MEMTYPE_DEFECTIVE 5     // Memory marked as defective.

// All other MEMTYPE values should be considered "reserved"

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MB2_DEFS_H__ */

/********************************************************************************************************************/
