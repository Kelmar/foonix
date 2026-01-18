/*************************************************************************/
/*************************************************************************/

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/debug.h>
#include <kernel/vm/vm.h>

#include "cpu.h"
#include "multiboot.h"
#include "arch_vm.h"

/*************************************************************************/

// The assembly code will initialize this value for us.
multiboot_t *g_Multiboot;

/*************************************************************************/

int Multiboot::InitMultibootMemory(KernelArgs *ka)
{
    // We need to come up with some sort of memory map....

    if (!g_Multiboot)
    {
        Debug::PrintF("No multiboot structure provided.\r\n");
        return -1;
    }

    // This is really just a hint, we'll want to detect actual memory config later.
    ka->MemorySizeKByte = g_Multiboot->mem_lower + g_Multiboot->mem_upper;

    if ((g_Multiboot->flags & MB_FLAG_MEM) == 0)
    {
        Debug::PrintF("No memory map provided by multiboot.\r\n");
        return -1;
    }

    size_t recordCnt = g_Multiboot->mmap_length / sizeof(mb_memory_map_t);
    bool processing = true;
    
    for (uint32_t i = 0; processing && i < recordCnt; ++i)
    {
        mb_memory_map_t *record = &g_Multiboot->mmap_addr[i];

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
