/********************************************************************************************************************/
/*
 * Code for reading Multiboot1 information.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/boot_args.h>
#include <kernel/kernel.h>

#include "multiboot.h"

/********************************************************************************************************************/

using namespace boot;


// Multiboot 1 doesn't specify a max size for the command line.
// Here we pick a (hopefully) sane default.
constexpr const size_t MAX_CMD_LINE = 256;

/********************************************************************************************************************/

static
void ParseCommandLine(ArgumentData *argData, multiboot_t *multi)
{
    if ((multi->flags & MB_FLAG_CMDLINE) == 0)
        return; // No command line.

    const char *cmd = reinterpret_cast<const char *>(multi->cmdline);
    strncpy(argData->CommandLine, cmd, ArgumentData::MaxCommandLine);
}

/********************************************************************************************************************/

static
void ParseFrameBufferInfo(ArgumentData *argData, multiboot_t *multi)
{
    if ((multi->flags & MB_FLAG_VBE) == 0)
        return;

    
}

/********************************************************************************************************************/

static
void ParseBasicMemoryInfo(ArgumentData *argData, multiboot_t *multi)
{
    if ((multi->flags & MB_FLAG_MEM) == 0)
    {
        //Debug::PrintF("No basic memory info provided by multiboot.");
        return;
    }

    // This is really just a hint, we'll want to detect actual memory config later.
    argData->LowMemorySizeKByte = multi->mem_lower;
    argData->HighMemorySizeKByte = multi->mem_upper;
}

/********************************************************************************************************************/

static
int ParseMemoryMap(ArgumentData *argData, multiboot_t *multi)
{
    if ((multi->flags & MB_FLAG_MMAP) == 0)
        return -1;

    size_t recordCnt = multi->mmap_length / sizeof(mb_memory_map_t);
    bool processing = true;

    mb_memory_map_t *memMap = reinterpret_cast<mb_memory_map_t *>(multi->mmap_addr);

    //Debug::PrintF("Checking %d memory record(s) from multiboot.\r\n", recordCnt);

    size_t biIndex = 0;
    
    for (uint32_t i = 0; processing && i < recordCnt; ++i)
    {
        mb_memory_map_t *record = &memMap[i];

        if (record->type != MemoryType::Available)
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
        argData->MemoryMap[biIndex].Start = record->base_addr;
        argData->MemoryMap[biIndex].Length = record->length;
        argData->MemoryMap[biIndex].Type = record->type;

        if (++biIndex >= ArgumentData::MaxMemoryEntries)
            break; // We've run out fo space in the memory map table.
    }

    argData->MemoryMapCount = biIndex;

    //Debug::PrintF("Multiboot memory read complete.\r\n");

    return 0;
}

/********************************************************************************************************************/

int Multiboot::ReadInfo(ArgumentData *argData, uint32_t multiboot_ptr)
{
    multiboot_t *multi = reinterpret_cast<multiboot_t *>(multiboot_ptr);

    //Debug::PrintF("Multiboot Info: %p\r\n", multi);

    ParseCommandLine(argData, multi);
    ParseFrameBufferInfo(argData, multi);
    ParseBasicMemoryInfo(argData, multi);

    return ParseMemoryMap(argData, multi);
}

/********************************************************************************************************************/
