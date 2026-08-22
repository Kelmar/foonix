/********************************************************************************************************************/
/*
 * Code for reading Multiboot1 information.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/debug.h>

#include "bootinfo.h"

/********************************************************************************************************************/

namespace
{
    // Multiboot 1 doesn't specify a max size for the command line.
    // Here we pick a (hopefully) sane default.
    constexpr const size_t MAX_CMD_LINE = 256;

    /************************************************************************************************************/

    void ParseCommandLine(BootInfo *bootInfo, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_CMDLINE) == 0)
            return; // No command line.

        const char *cmd = reinterpret_cast<const char *>(multi->cmdline);
        strncpy(bootInfo->CommandLine, cmd, BootInfo::CmdLineSize);
    }

    /************************************************************************************************************/

    void ParseBasicMemoryInfo(BootInfo *bootInfo, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_MEM) == 0)
        {
            //Debug::PrintF("No basic memory info provided by multiboot.");
            return;
        }

        // This is really just a hint, we'll want to detect actual memory config later.
        bootInfo->LowMemorySize = multi->mem_lower;
        bootInfo->HighMemorySize = multi->mem_upper;
    }

    /************************************************************************************************************/

    int ParseMemoryMap(BootInfo *bootInfo, multiboot_t *multi)
    {
        if ((multi->flags & MB_FLAG_MMAP) == 0)
            return -1;

        size_t recordCnt = multi->mmap_length / sizeof(mb_memory_map_t);
        bool processing = true;

        mb_memory_map_t *memMap = reinterpret_cast<mb_memory_map_t *>(multi->mmap_addr);

        Debug::PrintF("Checking %d memory record(s) from multiboot.\r\n", recordCnt);

        size_t biIndex = 0;
        
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
            bootInfo->MemoryInfo[biIndex].Start = record->base_addr;
            bootInfo->MemoryInfo[biIndex].Length = record->length;
            bootInfo->MemoryInfo[biIndex].Type = record->type;

            if (++biIndex >= BootInfo::MaxMemArgs)
                break; // We've run out fo space in the memory map table.
        }

        bootInfo->MemoryInfoCount = biIndex;

        Debug::PrintF("Multiboot memory read complete.\r\n");

        return 0;
    }
}

/********************************************************************************************************************/

int Multiboot::ReadInfo(BootInfo *bootInfo, uint32_t multiboot_ptr)
{
    multiboot_t *multi = reinterpret_cast<multiboot_t *>(multiboot_ptr);

    Debug::PrintF("Multiboot Info: %p\r\n", multi);
    
    ParseCommandLine(bootInfo, multi);
    ParseBasicMemoryInfo(bootInfo, multi);

    return ParseMemoryMap(bootInfo, multi);
}

/********************************************************************************************************************/
