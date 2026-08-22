/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef FOONIX_ARCH_X86_BOOTINFO_H__
#define FOONIX_ARCH_X86_BOOTINFO_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/types.h>

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

struct BootMemoryInfo
{
    uintptr_t Start;
    size_t Length;
    BiosMemoryType Type;
};

struct BootInfo
{
    static constexpr const size_t CmdLineSize = 1024;
    static constexpr const size_t MaxMemArgs = 128;

    /// @brief Our own copy of the command line saved in a safe place.
    char CommandLine[CmdLineSize];

    /// @brief Boot magic number (the type of boot loader we were started from)
    uint32_t BootMagicNumber;

    /// @brief The amount of memory available in the lower 1MB of memory in KB.
    size_t LowMemorySize;

    /// @brief The amount of memory available above 1MB in KB.
    size_t HighMemorySize;

    /**
     * @brief Where the kernel starts in physical memory.
     *
     * @remarks Not aligned on a page boundary!
     */ 
    paddr_t KernelStart;

    /**
     * @brief Where the kernel ends in physical memory.
     *
     * @remarks Not aligned on a page boundary!
     */
    paddr_t KernelEnd;

    /// @brief Start of kernel heap in physical memory.
    paddr_t HeapStart;

    /**
     * @brief The next available phyiscal page.
     *
     * @remarks This is used to track allocations done by the startup bump allocator; which
     * we use for allocating the initial page tables.
     */ 
    paddr_t HeapNext;

    /// @brief Number of valid entries in the MemoryInfo table.
    size_t MemoryInfoCount;

    /// @brief Map of memory information supplied by the boot loader.
    BootMemoryInfo MemoryInfo[MaxMemArgs];

    /// @brief Physical address of boot PageTable object.
    //PageTable *BootPageTable;
};

/********************************************************************************************************************/

#include "multiboot.h"
#include "multiboot2.h"

/********************************************************************************************************************/

namespace Multiboot
{
    int ReadInfo(BootInfo *bi, uint32_t multiboot_ptr);
}

namespace MB2
{
    int ReadInfo(BootInfo *bi, uint32_t multiboot_ptr);
}

/********************************************************************************************************************/

#endif /* FOONIX_ARCH_X86_BOOTINFO_H__ */

/********************************************************************************************************************/
