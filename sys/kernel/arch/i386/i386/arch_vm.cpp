/*************************************************************************/
/*************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/vm/vm.h>

#include "cpu.h"
#include "multiboot.h"
#include "arch_vm.h"

/*************************************************************************/

// Symbols defined by linker script
extern "C" uintptr_t kernel_start; // Located at a physical address
extern "C" uintptr_t kernel_end;   // Located at a logical address

/*************************************************************************/

enum class BootMethod : uint32_t
{
    /// @brief Booted with an unknown bootloader, expect wild wild west.
    Unknown   = 0,

    /// @brief Booted via a Multiboot loader, expect g_Multiboot to be filled out.
    Multiboot = 1,

    /// @brief Booted via (U)EFI system    
    EFI       = 2,
};

BootMethod g_BootMethod;

/********************************************************************************************************************/

namespace
{
    // Compute space for a map of the lower 1MB of pages.
    // We track these pages as special identity mapped pages.
    const size_t LOWER_1MB        = 1024 * 1024;
    const size_t DWORD_PAGES      = sizeof(uint32_t) * 8;
    const size_t LOWER_1MB_BITS   = LOWER_1MB / PAGE_SIZE;
    const size_t LOWER_1MB_BYTES  = LOWER_1MB_BITS / 8;
    const size_t LOWER_1MB_DWORDS = LOWER_1MB_BYTES / sizeof(uint32_t);

    uint32_t s_RealMemMap[LOWER_1MB_DWORDS];
}

/********************************************************************************************************************/

void Arch::VM::ReserveRealMemory(physical_addr_t addr, size_t length)
{
    if (addr >= LOWER_1MB)
        return;

    if ((addr + length) >= LOWER_1MB)
        length = LOWER_1MB - addr;

    // Make sure we start on a page boundary.
    uint64_t pageNumber = addr / PAGE_SIZE;

    // Tell the VM these pages have been reserved.
    size_t count = (length + (PAGE_SIZE - 1)) / PAGE_SIZE;

    for (size_t i = 0; i < count; ++i, ++pageNumber)
    {
        size_t pageIndex = pageNumber / (sizeof(uint32_t) * 8);
        size_t pageOffset = pageNumber % (sizeof(uint32_t) * 8);

        s_RealMemMap[pageIndex] |= 1 << pageOffset;
    }
}

/********************************************************************************************************************/

physical_addr_t Arch::VM::AllocRealMemory(void)
{
    uint32_t mapWord;
    size_t index = -1;
    size_t offset = 0;

    for (;;)
    {
        do
        {
            mapWord = s_RealMemMap[++index];
        } while (mapWord == 0xFFFFFFFF && index < LOWER_1MB_DWORDS);

        if (index >= LOWER_1MB_DWORDS)
            return 0; // No available memory!

        for (offset = 0; offset < 8; ++offset)
        {
            int bit = 1 << offset;

            if ((mapWord & bit) == 0)
            {
                s_RealMemMap[index] |= bit;
                break;
            }
        }

        if (offset < 8)
            return (index * DWORD_PAGES) + offset;
    }
}

/********************************************************************************************************************/

void Arch::VM::ReleaseRealMemory(physical_addr_t addr)
{
    int index = addr / DWORD_PAGES;
    int offset = addr % DWORD_PAGES;

    if (index == 0 || offset == 0)
        return; // Do not release the NULL page!

    s_RealMemMap[index] &= ~(1 << offset);
}

/********************************************************************************************************************/

void Arch::InitBootMemory(KernelArgs *ka)
{
    Debug::PrintF("ENTER: Arch::InitBootMemory()\r\n");

    Debug::PrintF("Boot Method: %d\r\n", g_BootMethod);

    // Figure out where we live in physical memory.
    ka->KernelCode.Base = reinterpret_cast<uintptr_t>(&kernel_start);
    uintptr_t kend = reinterpret_cast<uintptr_t>(&kernel_end);

    kend -= RELOCATE_START; // Map kernel end to physical page
;
    ka->KernelCode.Length = kend - ka->KernelCode.Base;

    Debug::PrintF("Kernel: %p %08X\r\n", ka->KernelCode.Base, ka->KernelCode.Length);

    int err;

    switch (g_BootMethod)
    {
    case BootMethod::Multiboot:
        err = Multiboot::InitMultibootMemory(ka);
        break;

    case BootMethod::Unknown:
    default:
        // TODO: Fallback to BIOS probe
        err = -1;
        break;
    }

    if (err)
        Debug::PrintF("WARN: No memory map, guessing.\r\n");



    // Mark the first 1MB as reserved with the VM, we manage those ourselves.
    //::VM::ReserveBootPages(0, 256);
}

/*************************************************************************/

