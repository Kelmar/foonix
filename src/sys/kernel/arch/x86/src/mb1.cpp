/********************************************************************************************************************/
/*
 * Code for reading Multiboot1 information.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/utilities.h>
#include <kernel/vm.h>

#include "asm.h"
#include "cpu.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "bootinfo.h"

//#include "page_table.h" // x64 version
//#include "page.h"

/********************************************************************************************************************/

namespace
{
    // Multiboot 1 doesn't specify a max size for the command line.
    // Here we pick a (hopefully) sane default.
    constexpr const size_t MAX_CMD_LINE = 256;

    void ParseCommandLine(KernelArgs *ka, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_CMDLINE) == 0)
            return; // No command line.

        const char *cmd = reinterpret_cast<const char *>(multi->cmdline);
        
        ka->SetCommandLine(cmd, MAX_CMD_LINE);
    }

    void ParseBasicMemoryInfo(KernelArgs *ka, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_MEM) == 0)
        {
            // TODO: Guess at what the memory map is based on the kernel's physical address.
            Debug::PrintF("No basic memory info provided by multiboot.");
            return;
        }

        // This is really just a hint, we'll want to detect actual memory config later.
        ka->MemorySizeKByte = multi->mem_lower + multi->mem_upper;

        Debug::PrintF("Multiboot reports %d KBytes available.\r\n", ka->MemorySizeKByte);
    }

    int ParseMemoryMap(KernelArgs *ka, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_MMAP) == 0)
        {
            Debug::PrintF("No memory map provided by multiboot.\r\n");
            return -1;
        }

        size_t recordCnt = multi->mmap_length / sizeof(mb_memory_map_t);
        bool processing = true;

        mb_memory_map_t *memMap = reinterpret_cast<mb_memory_map_t *>(multi->mmap_addr);

        Debug::PrintF("Checking %d memory record(s) from multiboot.\r\n", recordCnt);
        
        for (uint32_t i = 0; processing && i < recordCnt; ++i)
        {
            mb_memory_map_t *record = &memMap[i];

            if (record->type != BiosMemoryType::Available)
            {
                // Ignore anything that isn't marked as available.
                continue;
            }

#if 0
            if (record->base_addr > MAX_32_ADDR)
            {
                // Don't think records will show up out of order, but we keep going, just in case.
                continue; 
            }
#endif 

            processing &= ka->AddMemoryMap(record->base_addr, record->length);
        }

        Debug::PrintF("Removing kernel usage from memory map.\r\n");

        // Remove any memory used by boot loader (e.g. Kernel code space)
        ka->KnockoutUsedMemory();

        Debug::PrintF("Multiboot memory read complete.\r\n");

        return 0;
    }
}

/********************************************************************************************************************/

int Multiboot::ReadInfo(KernelArgs *ka, uint32_t multiboot_ptr)
{
    multiboot_t *multi = reinterpret_cast<multiboot_t *>(multiboot_ptr);

    Debug::PrintF("Multiboot Info: %p\r\n", multi);
    
    ParseCommandLine(ka, multi);
    ParseBasicMemoryInfo(ka, multi);

    return ParseMemoryMap(ka, multi);
}

/********************************************************************************************************************/
