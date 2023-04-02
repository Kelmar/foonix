/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MULTBOOT_H__
#define __FOONIX_KERNEL_MULTBOOT_H__

/********************************************************************************************************************/

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

struct mb_memory_map_t
{
    uint32_t size;      // Size of this structure
    uint64_t base_addr; // Memory start address
    uint64_t length;    // Memory end address
    uint32_t type;      // 1 == usable (everything else unusable)
} __attribute__((packed));

/********************************************************************************************************************/

struct multiboot_t
{
    uint32_t flags;

#define MB_FLAG_MEM	0x00000001
    uint32_t mem_lower;	/* Amount of available lower memory in KBytes (640K) */
    uint32_t mem_upper; /* Amount of available upper memory in KBytes (1M) */

#define MB_FLAG_BOOTDEV	0x00000002

#define MB_BOOT_DRIVE 0
#define MB_BOOT_PART1 1 /* Primary partition (includes DOS ext, starting at 4)*/
#define MB_BOOT_PART2 2 /* "Sub" partition, (BSD partitions) */
#define MB_BOOT_PART3 3 /* Sub-Sub partition.... */
    uint8_t boot_device[4];

#define MB_FLAG_CMDLINE	0x00000004
    uint32_t cmdline; /* C-style null termianted string command line args. */

#define MB_FLAG_MODS	0x00000008
    uint32_t mods_count; /* Number of loaded "modules" */
    uint32_t mods_addr;  /* Physical address of first module entry. */

#define MB_FLAG_AOUTSYMS 0x00000010
#define MB_FLAG_ELFSYMS	 0x00000020
    union
    {
        mb_aout_syms_t aout_syms;
        mb_elf_syms_t elf_syms;
    } __attribute__((packed));

#define MB_FLAG_MMAP	0x00000040
    uint32_t mmap_length; /* in bytes */
    mb_memory_map_t *mmap_addr;

#define MB_FLAG_DRIVERS	0x00000080
    uint32_t drives_length;
    uint32_t drives_addr;

#define MB_FLAG_CONFIG	0x00000100
    uint32_t config_table;

#define MB_FLAG_BLNAME	0x00000200
    uint32_t boot_loader_name;

#define MB_FLAG_APM	0x00000400
    uint32_t apm_table;

#define MB_FLAG_VBE	0x00000800
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} __attribute__((packed));

/********************************************************************************************************************/

namespace Multiboot
{
    void InitMultibootMemory(KernelArgs *);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MULTBOOT_H__ */

/********************************************************************************************************************/
