/********************************************************************************************************************/
/*
 * Code for reading Multiboot2 information.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <algorithm>

#include <kernel/boot_args.h>
#include <kernel/kernel.h>

#include <kernel/utilities.h>
#include <kernel/utils/span.h>

#include "multiboot2.h"

/********************************************************************************************************************/

using namespace boot;

/********************************************************************************************************************/

static
int ParseCommandLine(ArgumentData *argData, const mb2_tag *tag)
{
    const mb2_str_tag *cmd = reinterpret_cast<const mb2_str_tag *>(tag);

    util::span<char> cmdStr = cmd->GetStr();

    size_t sz = std::min(cmdStr.size_bytes(), ArgumentData::MaxCommandLine);

    if (sz == 0)
        return 0; // Nothing to copy.

    --sz; // Ensure space for null terminator.

    memcpy(argData->CommandLine, cmdStr.data(), sz);
    argData->CommandLine[sz] = '\0';

    return 0;
}

/********************************************************************************************************************/

static
int ParseBasicMemoryInfo(ArgumentData *argData, const mb2_tag *tag)
{
    const mb2_basic_memory_info *info = reinterpret_cast<const mb2_basic_memory_info *>(tag);

    argData->LowMemorySizeKByte = info->mem_lower;
    argData->HighMemorySizeKByte = info->mem_upper;

    return 0;
}

/********************************************************************************************************************/

static
int ParseMemoryMap(ArgumentData *argData, const mb2_tag *tag)
{
    const mb2_memory_info *info = reinterpret_cast<const mb2_memory_info *>(tag);

    //Debug::PrintF("Checking %d memory record(s) from multiboot 2.\r\n", info->GetExtent());

    int biIndex = 0;

    for (auto item : info->GetEntries())
    {
        if (item.type != MB2MemoryType::Available)
            continue; // Skip anything we can't use for boot up.

        argData->MemoryMap[biIndex].Start = item.base_addr;
        argData->MemoryMap[biIndex].Length = item.length;
        argData->MemoryMap[biIndex].Type = static_cast<MemoryType>(item.type);

        if (++biIndex >= ArgumentData::MaxMemoryEntries)
            break; // We've run out fo space in the memory map table.
    }

    argData->MemoryMapCount = biIndex;

    //Debug::PrintF("Multiboot memory read complete.\r\n");

    return 0;
}

/********************************************************************************************************************/

int MB2::ReadInfo(ArgumentData *argData, uint32_t multiboot_ptr)
{
    //Debug::PrintF("Multiboot 2 load detected.\r\n");

    auto info = reinterpret_cast<mb2_info *>(multiboot_ptr);

    //Debug::PrintF("MB2 info @%p size: %d\r\n", info, info->total_size);

    uintptr_t ptr = multiboot_ptr + sizeof(mb2_info);

    size_t sz = sizeof(mb2_info);
    int err = 0;  //kernel::ErrorCode::NoError;
    //int i = 0;
    
    while (sz < info->total_size)
    {
        //++i;

        // MB2 tags should be aligned on 8-byte boundaries.
        uintptr_t p2 = util::AlignCeiling<8>(ptr);
        size_t diff = p2 - ptr;

        sz += diff;
        ptr = p2;

        auto tag = reinterpret_cast<mb2_tag *>(ptr);
        //Debug::PrintF("Tag %d: T(%d) SZ(%d)\r\n", i, tag->type, tag->size);

        if (tag->type == MB2_TAG_END)
        {
            //if (tag->size != 8)
                //Debug::PrintF("WARN: Saw NULL tag on MB2 structure with invalid size.\r\n");

            break;
        }

        switch (tag->type)
        {
        case MB2_TAG_BOOT_CMD:
            err = ParseCommandLine(argData, tag);
            break;

        case MB2_TAG_BASIC_MEMINFO:
            err = ParseBasicMemoryInfo(argData, tag);
            break;

        case MB2_TAG_MEMORY_MAP:
            err = ParseMemoryMap(argData, tag);
            break;

        default:
            err = 0; //kernel::ErrorCode::NoError;
            break;
        }

        if (err != 0)
        {
            // Not sure what to do about this just yet.
        }

        sz += tag->size;
        ptr += tag->size;
    }

    //Debug::PrintF("MB2: Read completed, processed %d total tags.\r\n", i);

    return err;
}

/********************************************************************************************************************/
