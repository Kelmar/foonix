/********************************************************************************************************************/
/*
 * Multiboot 1 definitions and structures.
 *
 * Used by C++, not suitable for ASM inclusion.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MULTBOOT_H__
#define __FOONIX_KERNEL_MULTBOOT_H__

/********************************************************************************************************************/

#include "mb_defs.h"

#include <stdint.h>

#include <kernel/kernel_args.h>

/********************************************************************************************************************/
/*
 * NOTE:
 * We can't use "real" pointers because the multiboot loader specification uses a 32 bit pointer for the structure.
 * The bootloader also will not put us into the 64 bit mode, rather a 32 bit mode.
 *
 * If we wish to later develop a 64 bit OS, we will need to translate these 32 bit pointers into 64 bit space at
 * some point.
 */
 /********************************************************************************************************************/

struct mb_module_t
{
    uint32_t mod_start; /* Physical starting address of module. */
    uint32_t mod_end;	/* Physical ending address of module. */
    uint32_t string;	/* OS specific string */
    uint32_t reserved;	/* Unused, should be zero */
} __attribute__((packed));

/********************************************************************************************************************/

struct mb_aout_syms_t
{
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;	/* Address of the symbols. */
    uint32_t reserved;	/* Reserved, should be zero */
} __attribute__((packed));

struct mb_elf_syms_t
{
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} __attribute__((packed));

/********************************************************************************************************************/

enum class BiosMemoryType : uint32_t
{
    /// @brief Invalid entry
    Invalid   = 0,

    /// @brief Memory is available for use
    Available = 1,

    /// @brief Memory has been reserved by the BIOS
    Reserved  = 2,

    /// @brief Memory is used for ACPI
    ACPI      = 3,

    /// @brief Memory is used for ACPI NVS
    ACPI_VMS  = 4,

    /// @brief Memory has been marked as bad
    BadMemory = 5
};

struct mb_memory_map_t
{
    uint32_t size;      // Size of this structure
    uint64_t base_addr; // Memory start address
    uint64_t length;    // Memory end address
    BiosMemoryType type;      // 1 == usable (everything else unusable)
} __attribute__((packed));

/********************************************************************************************************************/

struct multiboot_t
{
    uint32_t flags;

// MB_FLAG_MEM
    uint32_t mem_lower;	/* Amount of available lower memory in KBytes (640K) */
    uint32_t mem_upper; /* Amount of available upper memory in KBytes (1M) */

// MB_FLAG_BOOTDEV (See also MB_BOOT_xxxx)
    uint8_t boot_device[4];

// MB_FLAG_CMDLINE
    uint32_t cmdline; /* C-style null terminated string command line args. */

// MB_FLAG_MODS
    uint32_t mods_count; /* Number of loaded "modules" */
    uint32_t mods_addr;  /* Physical address of first module entry. */

    union
    {
        mb_aout_syms_t aout_syms; // MB_FLAG_AOUTSYMS
        mb_elf_syms_t elf_syms;   // MB_FLAG_ELFSYMS
    } __attribute__((packed));

// MB_FLAG_MMAP
    uint32_t mmap_length; /* in bytes */
    mb_memory_map_t *mmap_addr;

// MB_FLAG_DRIVERS
    uint32_t drives_length;
    uint32_t drives_addr;

// MB_FLAG_CONFIG
    uint32_t config_table;

// MB_FLAG_BLNAME
    uint32_t boot_loader_name;

// MB_FLAG_APM
    uint32_t apm_table;

// MB_FLAG_VBE
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} __attribute__((packed));

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MULTBOOT_H__ */

/********************************************************************************************************************/
