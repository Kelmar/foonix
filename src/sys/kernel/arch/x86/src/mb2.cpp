/********************************************************************************************************************/
/*
 * Code for reading Multiboot2 information.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <algorithm>

#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/span.h>
#include <kernel/utilities.h>

#include "bootinfo.h"
#include "multiboot2.h"

/********************************************************************************************************************/

namespace
{
    int ParseCommandLine(BootInfo *bootInfo, const mb2_tag *tag)
    {
        const mb2_str_tag *cmd = reinterpret_cast<const mb2_str_tag *>(tag);

        util::span<char> cmdStr = cmd->GetStr();

        size_t sz = std::min(cmdStr.size_bytes(), BootInfo::CmdLineSize);

        if (sz == 0)
            return 0; // Nothing to copy.

        --sz; // Ensure space for null terminator.

        memcpy(bootInfo->CommandLine, cmdStr.data(), sz);
        bootInfo->CommandLine[sz] = '\0';

        return 0;
    }

    /************************************************************************************************************/

    int ParseBasicMemoryInfo(BootInfo *bootInfo, const mb2_tag *tag)
    {
        const mb2_basic_memory_info *info = reinterpret_cast<const mb2_basic_memory_info *>(tag);

        bootInfo->LowMemorySize = info->mem_lower;
        bootInfo->HighMemorySize = info->mem_upper;

        return 0;
    }

    /************************************************************************************************************/

    int ParseMemoryMap(BootInfo *bootInfo, const mb2_tag *tag)
    {
        const mb2_memory_info *info = reinterpret_cast<const mb2_memory_info *>(tag);

        //Debug::PrintF("Checking %d memory record(s) from multiboot 2.\r\n", info->GetExtent());

        int biIndex = 0;

        for (auto item : info->GetEntries())
        {
            if (item.type != MB2MemoryType::Available)
                continue; // Skip anything we can't use for boot up.

            bootInfo->MemoryInfo[biIndex].Start = item.base_addr;
            bootInfo->MemoryInfo[biIndex].Length = item.length;
            bootInfo->MemoryInfo[biIndex].Type = static_cast<BiosMemoryType>(item.type);

            if (++biIndex >= BootInfo::MaxMemArgs)
                break; // We've run out fo space in the memory map table.
        }

        bootInfo->MemoryInfoCount = biIndex;

        //Debug::PrintF("Multiboot memory read complete.\r\n");

        return 0;
    }
}

/********************************************************************************************************************/

int MB2::ReadInfo(BootInfo *bootInfo, uint32_t multiboot_ptr)
{
    Debug::PrintF("Multiboot 2 load detected.\r\n");

    auto info = reinterpret_cast<mb2_info *>(multiboot_ptr);

    Debug::PrintF("MB2 info @%p size: %d\r\n", info, info->total_size);

    uintptr_t ptr = multiboot_ptr + sizeof(mb2_info);

    size_t sz = sizeof(mb2_info);
    int err = 0;  //Kernel::ErrorCode::NoError;
    int i = 0;
    
    while (sz < info->total_size)
    {
        ++i;

        // MB2 tags should be aligned on 8-byte boundaries.
        uintptr_t p2 = util::AlignCeiling<8>(ptr);
        size_t diff = p2 - ptr;

        sz += diff;
        ptr = p2;

        auto tag = reinterpret_cast<mb2_tag *>(ptr);
        //Debug::PrintF("Tag %d: T(%d) SZ(%d)\r\n", i, tag->type, tag->size);

        if (tag->type == MB2_TAG_END)
        {
            if (tag->size != 8)
                Debug::PrintF("WARN: Saw NULL tag on MB2 structure with invalid size.\r\n");

            break;
        }

        switch (tag->type)
        {
        case MB2_TAG_BOOT_CMD:
            err = ParseCommandLine(bootInfo, tag);
            break;

        case MB2_TAG_BASIC_MEMINFO:
            err = ParseBasicMemoryInfo(bootInfo, tag);
            break;

        case MB2_TAG_MEMORY_MAP:
            err = ParseMemoryMap(bootInfo, tag);
            break;

        default:
            //Debug::PrintF("Ignoring, unknown tag type %d\r\n", tag->type);
            err = 0; //Kernel::ErrorCode::NoError;
            break;
        }

        if (err != 0)
        {
            // Not sure what to do about this just yet.
        }

        sz += tag->size;
        ptr += tag->size;
    }

    Debug::PrintF("MB2: Read completed, processed %d total tags.\r\n", i);

    return err;
}

/********************************************************************************************************************/
