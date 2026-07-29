/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/bitmap.h>
#include <kernel/debug.h>

#include <kernel/vm.h>
#include <kernel/arch.h>

#include "asm.h"
#include "cpu.h"
#include "multiboot.h"
#include "arch_vm.h"
#include "page.h"

/********************************************************************************************************************/

// The assembly code will initialize these values for us.
uint32_t g_BootMagic; /* Value from EAX register */
uint32_t g_Multiboot; /* Value from EBX register */

/********************************************************************************************************************/

int Multiboot::InitMultibootMemory(KernelArgs *ka)
{
    // We need to come up with some sort of memory map....
   
    // Remap the multiboot structure into virtual memory space.
    multiboot_t *multi = reinterpret_cast<multiboot_t *>(PHYS_2_VIRT(g_Multiboot));

    auto result = paging::g_bootPageTable.MapStruct(g_Multiboot, multi, 0);

    if (result != Kernel::ErrorCode::NoError)
        kpanic("Unable to map multiboot structure into page table.");

    Debug::PrintF("Multiboot Info: %p\r\n", multi);

    // This is really just a hint, we'll want to detect actual memory config later.
    ka->MemorySizeKByte = multi->mem_lower + multi->mem_upper;

    Debug::PrintF("Multiboot reports %d KBytes available.\r\n", ka->MemorySizeKByte);

    if ((multi->flags & MB_FLAG_MEM) == 0)
    {
        Debug::PrintF("No memory map provided by multiboot.\r\n");
        return -1;
    }

    size_t recordCnt = multi->mmap_length / sizeof(mb_memory_map_t);
    bool processing = true;

    mb_memory_map_t *memMap = reinterpret_cast<mb_memory_map_t *>(PHYS_2_VIRT(multi->mmap_addr));
    paddr_t addr = reinterpret_cast<paddr_t>(reinterpret_cast<uintptr_t>(multi->mmap_addr));

    result = paging::g_bootPageTable.MapStruct(addr, memMap, 0);

    if (result != Kernel::ErrorCode::NoError)
        kpanic("Unable to map multiboot memory map into page table.");

    Debug::PrintF("Checking %d memory record(s) from multiboot.\r\n", recordCnt);
    
    for (uint32_t i = 0; processing && i < recordCnt; ++i)
    {
        //mb_memory_map_t *record = &multi->mmap_addr[i];
        mb_memory_map_t *record = &memMap[i];

        if (record->type != BiosMemoryType::Available)
        {
            // Ignore anything that isn't marked as available.
            continue;
        }

        if (record->base_addr > MAX_32_ADDR)
        {
            // Don't think records will show up out of order, but we keep going, just in case.
            continue; 
        }

        processing &= ka->AddMemoryMap(record->base_addr, record->length);
    }

    Debug::PrintF("Removing kernel usage from memory map.\r\n");

    // Remove any memory used by boot loader (e.g. Kernel code space)
    ka->KnockoutUsedMemory();

    Debug::PrintF("Multiboot memory read complete.\r\n");

    return 0;
}

/********************************************************************************************************************/
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
    // Uses a simple bitmap allocation mark and free for the first 1MB of real memory pages.

    // Compute space for a map of the lower 1MB of pages.
    // We track these pages as special identity mapped pages.
    const size_t LOWER_1MB       = 1024 * 1024;
    const size_t LOWER_1MB_PAGES = LOWER_1MB / PAGE_SIZE;

    Bitmap<LOWER_1MB_PAGES> s_realMemMap;
}

/********************************************************************************************************************/
/*
 * Unconditionally marks a range of pages of real mode memory as in use.
 */
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
        //s_realMemMap[pageNumber] = true; // Ideal
        s_realMemMap.Set(pageNumber);
    }
}

/********************************************************************************************************************/
/*
 * Allocates a page of real memory.
 *
 * A return of zero indicates that we weren't able to allocate a real memory page.
 *
 * Note that this function will never attempt to allocate the first page, leaving that reserved for a null pointer.
 *
 * To get at this page use the above ReserveRealMemory() function.
 */
paddr_t Arch::VM::AllocRealMemory(void)
{
    size_t index = 0; // See note about 0 index.

    // Find first free page.
    while (s_realMemMap[++index])
        ;

    if (index >= LOWER_1MB_PAGES)
        return 0; // No available memory!

    return (index * PAGE_SIZE);
}

/********************************************************************************************************************/

void Arch::VM::ReleaseRealMemory(paddr_t addr)
{
    if (addr == 0)
    {
        /*
         * We're not panicking here, as there could be a legitimate reason to pass nullptr into this function.
         *
         * This mimics the behaviour of free() and delete with a nullptr.
         *
         * Note that this also prevents the first page from ever being freed.  This is actually an ideal side
         * effect to ensure things don't try to use it, leaving it reserved indicating it's special status.
         */

        return; // Do not release the NULL page!
    }

    size_t index = (addr / PAGE_SIZE);

    //s_realMemMap[index] = false; // Ideal
    s_realMemMap.Clear(index);
}

/********************************************************************************************************************/

void arch::InitBootMemory(KernelArgs *ka)
{
    Debug::PrintF("ENTER: Arch::InitBootMemory()\r\n");

    Debug::PrintF("Boot Magic: 0x%08X\r\n", g_BootMagic);

    // Figure out where we live in physical memory.
    ka->KernelCode.Base = reinterpret_cast<uintptr_t>(&kernel_start);
    uintptr_t kend = VIRT_2_PHYS(reinterpret_cast<uintptr_t>(&kernel_end));

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
}

/********************************************************************************************************************/

