/********************************************************************************************************************/
/*
 * Multiboot 2 definitions and structures.
 *
 * Used by C++, not suitable for ASM inclusion.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MULTBOOT2_H__
#define __FOONIX_KERNEL_MULTBOOT2_H__

/********************************************************************************************************************/

#include "mb2_defs.h"

#include <stdint.h>

#include <kernel/span.h>

/********************************************************************************************************************/

struct mb2_tag
{
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct mb2_info
{
    uint32_t total_size;
    uint32_t reserved;

    // Tags immediately follow
} __attribute__((packed));

// MB2_TAG_BOOT_CMD (1), MB2_TAG_BOOTLOADER (2)
struct mb2_str_tag : mb2_tag
{
    size_t StrSize() const { return size - sizeof(mb2_tag); }

    const util::span<char> GetStr() const
    {
        uintptr_t p = reinterpret_cast<uintptr_t>(this);
        size_t sz = StrSize();
        p += sz;
        return util::span<char>(reinterpret_cast<char *>(p), sz);
    }
} __attribute__((packed));

// MB2_TAG_BASIC_MEMINFO (4)
struct mb2_basic_memory_info : mb2_tag
{
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed));

// MB2_TAG_MEMORY_MAP (6)
struct mb2_memory_info : mb2_tag
{
    uint32_t entry_size;    // Size of one entry
    uint32_t entry_version; // Should always be zero

    struct entry
    {
        uint64_t base_addr; // Physical address
        uint64_t length;    // Length of memory range
        uint32_t type;
        uint32_t reserved;  // Not used, always zero
    };

    constexpr size_t GetExtent() const { return (size - sizeof(mb2_memory_info)) / entry_size; }

    const util::span<entry> GetEntries() const
    {
        uintptr_t p = reinterpret_cast<uintptr_t>(this);
        p += sizeof(mb2_memory_info);

        return util::span<entry>(reinterpret_cast<entry *>(p), GetExtent(), entry_size);
    }
} __attribute__((packed));

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MULTBOOT2_H__ */

/********************************************************************************************************************/
