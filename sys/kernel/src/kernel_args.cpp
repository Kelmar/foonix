/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/debug.h>

#include "cpu.h"

/********************************************************************************************************************/

bool KernelArgs::AddFreeMemory(paddr_t addr, size_t length)
{
    bool rval = true;

    if (MemoryMapEntries >= MAX_MEMORY_ENTRIES)
        return false;

    uint64_t end = addr + length;

    if (addr == KernelCode.Base)
    {
        // Ideal case, we're on a boundary.

        if (KernelCode.Length >= length)
        {
            // We take up the whole record, just ignore it.
            return true;
        }
        
        // Shorten the record and continue.
        length -= KernelCode.Length;
        addr += KernelCode.Length;
    }
    else if (addr > KernelCode.Base && addr < KernelCode.End())
    {
        // Record starts in the middle of the kernel.

        if (KernelCode.End() >= end)
        {
            // We take up the whole record, ignore it.
            return true;
        }

        // Adjust record and continue
        uint64_t klen = end - KernelCode.End();
        length -= klen;
        addr += klen;
    }
    else if (addr < KernelCode.Base && end > KernelCode.End())
    {
        // Kernel is inside this whole record, split it.

        paddr_t newAddr = addr;
        size_t newLen = KernelCode.Base - addr;

        // We can't get here without there already being an entry available, so ignore result.
        AddFreeMemory(newAddr, newLen);

        newAddr = KernelCode.End();
        newLen = newAddr - addr;

        return AddFreeMemory(newAddr, newLen);
    }
    
    if (end > MAX_ADDR)
    {
        // Truncate to end of 32-bit address space.
        length = MAX_ADDR - addr;

        // We've reached the end of what we can fit in our memory map.
        rval = false;
    }

    MemoryMap[MemoryMapEntries].Base = addr;
    MemoryMap[MemoryMapEntries].Length = length;

    ++MemoryMapEntries;

    return rval;
}

/********************************************************************************************************************/

void KernelArgs::ShowAvailableMemory(void)
{
    Debug::PrintF("Free Memory\r\n");

    for (uint32_t i = 0; i < MemoryMapEntries; ++i)
    {
        Debug::PrintF("    %p %08X\r\n", MemoryMap[i].Base, MemoryMap[i].Length);
    }
}

/********************************************************************************************************************/
