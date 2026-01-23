/*************************************************************************/
/*************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/vm/vm.h>

#include "asm.h"
#include "cpu.h"
#include "multiboot.h"
#include "arch_vm.h"

/*************************************************************************/

// The assembly code will initialize these values for us.
uint32_t g_BootMagic; /* Value from EAX register */
uint32_t g_Multiboot; /* Value from EBX register */

/*************************************************************************/

int Multiboot::InitMultibootMemory(KernelArgs *ka)
{
    // We need to come up with some sort of memory map....

    // Remap the multiboot structure into virtual memory space.
    multiboot_t *multi = reinterpret_cast<multiboot_t *>(PHYS_2_VIRT(g_Multiboot));
    Debug::PrintF("Multiboot Info: %p\r\n", multi);

    // This is really just a hint, we'll want to detect actual memory config later.
    ka->MemorySizeKByte = multi->mem_lower + multi->mem_upper;

    if ((multi->flags & MB_FLAG_MEM) == 0)
    {
        Debug::PrintF("No memory map provided by multiboot.\r\n");
        return -1;
    }

    size_t recordCnt = multi->mmap_length / sizeof(mb_memory_map_t);
    bool processing = true;
    
    for (uint32_t i = 0; processing && i < recordCnt; ++i)
    {
        mb_memory_map_t *record = &multi->mmap_addr[i];

        if (record->type != BiosMemoryType::Available)
        {
            // Everything else we just mark as unavailable.
            continue;
        }

        if (record->base_addr > MAX_32_ADDR)
        {
            // Don't think records will show up out of order, but we keep going, just in case.
            continue; 
        }

        processing &= ka->AddFreeMemory(record->base_addr, record->length);
    }

    return 0;
}

/*************************************************************************/
/*
 * These are defined in the linker script.
 */

/// @brief Physical memory location of the start of the kernel.
extern "C" uintptr_t _kernel_phys_start;

/// @brief Physical memory location of the end of the kernel.
extern "C" uintptr_t _kernel_end;

constexpr void *kernel_start = &_kernel_phys_start;

constexpr void *kernel_end = &_kernel_end; //VIRT_2_PHYS(&_kernel_end);

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

void Arch::VM::ReserveRealMemory(paddr_t addr, size_t length)
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

paddr_t Arch::VM::AllocRealMemory(void)
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

void Arch::VM::ReleaseRealMemory(paddr_t addr)
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

    Debug::PrintF("Boot Magic: 0x%08X\r\n", g_BootMagic);

    // Figure out where we live in physical memory.
    ka->KernelCode.Base = reinterpret_cast<uintptr_t>(&kernel_start);
    uintptr_t kend = reinterpret_cast<uintptr_t>(&kernel_end);

    kend -= RELOCATE_START; // Map kernel end to physical page
;
    ka->KernelCode.Length = kend - ka->KernelCode.Base;

    Debug::PrintF("Kernel: %p %08X\r\n", ka->KernelCode.Base, ka->KernelCode.Length);

    int err;

    switch (g_BootMagic)
    {
    case MULTIBOOT_MAGIC:
        err = Multiboot::InitMultibootMemory(ka);
        break;

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

