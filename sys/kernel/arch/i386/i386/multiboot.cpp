/*************************************************************************/
/*************************************************************************/

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/debug.h>
#include <kernel/vm/vm.h>

#include "cpu.h"
#include "multiboot.h"

/*************************************************************************/

#define RELOCATE_START 0xC0000000

// The assembly code will initialize this value for us.
multiboot_t *g_Multiboot;

// Symbols defined by linker script
extern "C" uintptr_t kernel_start; // Located at a physical address
extern "C" uintptr_t kernel_head;  // Located at a logical address
extern "C" uintptr_t kernel_end;   // Located at a logical address

/*************************************************************************/

namespace
{
    void ReserveMemoryArea(addr_t start, size_t len)
    {
        if (start >= (4 * 1024 * 1024))
            return;

        // Make sure we start on a page boundary.
        addr_t addr = (start / PAGE_SIZE) * PAGE_SIZE;

        // Tell the VM these pages have been reserved.
        len = (len + (PAGE_SIZE - 1)) / PAGE_SIZE;
        VM::ReserveBootPages(addr, len);
    }
}

/*************************************************************************/

void Multiboot::InitMultibootMemory(KernelArgs *ka)
{
    // We need to come up with some sort of memory map....

    if (!g_Multiboot)
    {
        Debug::PrintF("No multiboot structure provided.\r\n");
        return;
    }

    if ((g_Multiboot->flags & MB_FLAG_MEM) == 0)
    {
        Debug::PrintF("No memory map provided by multiboot.\r\n");
        return;
    }

    size_t recordCnt = g_Multiboot->mmap_length / sizeof(mb_memory_map_t);
    uint32_t mapRecord = 0;
    bool processing = true;
    
    for (uint32_t i = 0; processing && i < recordCnt; ++i)
    {
        mb_memory_map_t *record = &g_Multiboot->mmap_addr[i];

        if (record->type != 1)
        {
            ReserveMemoryArea(record->base_addr, record->length);

            continue; // We only care about available memory types for now.
        }

        if (record->base_addr > MAX_32_ADDR)
        {
            // Don't think records will show up out of order, but we keep going, just in case.
            continue; 
        }

        uint64_t length = record->length;
        uint64_t end = record->base_addr + length;
        
        if (end > MAX_32_ADDR)
        {
            // Truncate to end of 32-bit address space.
            length = MAX_32_ADDR - record->base_addr;

            // We've reached the end of what we can fit in our memory map
            processing = false;
        }

        ka->MemoryMap[mapRecord].Base = (uint32_t)record->base_addr;
        ka->MemoryMap[mapRecord].Length = (uint32_t)length;

        ++mapRecord;

        processing &= mapRecord < KernelArgs::MAX_MEMORY_ENTRIES;
    }

    ka->MemoryMapEntries = mapRecord;

    /*
     * One thing that Multiboot apparently doesn't do for us is tell us that the memory
     * for our kernel is in use.  So the VM could pick a page that is being used and
     * stomp all over our code! >_<
     * 
     * Figure out where the kernel lives and do some magic so that doesn't happen.
     */ 
    bochs_breakpoint();

    uintptr_t kstart = reinterpret_cast<uintptr_t>(&kernel_start);
    uintptr_t kend = reinterpret_cast<uintptr_t>(&kernel_end);

    kend -= RELOCATE_START; // Map kernel end to physical page

    size_t len = kend - kstart;

    ReserveMemoryArea(kstart, len);


}

/*************************************************************************/
