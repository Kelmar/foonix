/********************************************************************************************************************/
/*
 * Code for reading Multiboot2 information.
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
    int ParseCommandLine(KernelArgs *ka, const mb2_tag *tag)
    {
        const mb2_str_tag *cmd = reinterpret_cast<const mb2_str_tag *>(tag);

        ka->SetCommandLine(cmd->GetStr());

        return 0;
    }

    int ParseBasicMemoryInfo(KernelArgs *ka, const mb2_tag *tag)
    {
        const mb2_basic_memory_info *info = reinterpret_cast<const mb2_basic_memory_info *>(tag);

        ka->MemorySizeKByte = info->mem_lower + info->mem_upper;

        Debug::PrintF("Multiboot reports %d KBytes available.\r\n", ka->MemorySizeKByte);

        return 0;
    }

    int ParseMemoryMap(KernelArgs *ka, const mb2_tag *tag)
    {
        const mb2_memory_info *info = reinterpret_cast<const mb2_memory_info *>(tag);

        Debug::PrintF("Checking %d memory record(s) from multiboot 2.\r\n", info->GetExtent());

        for (auto item : info->GetEntries())
        {
            if (item.type != MB2MemoryType::Available)
                continue; // Skip anything we can't use for boot up.

            paddr_t base = static_cast<paddr_t>(item.base_addr);

            if (!ka->AddMemoryMap(base, item.length))
                break; // We've run out of space in the memory map tables.
        }

        Debug::PrintF("Removing kernel usage from memory map.\r\n");

        // Remove any memory used by boot loader (e.g. Kernel code space)
        ka->KnockoutUsedMemory();

        Debug::PrintF("Multiboot memory read complete.\r\n");

        return 0;
    }
}

/********************************************************************************************************************/

int MB2::ReadInfo(KernelArgs *ka, uint32_t multiboot_ptr)
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
            err = ParseCommandLine(ka, tag);
            break;

        case MB2_TAG_BASIC_MEMINFO:
            err = ParseBasicMemoryInfo(ka, tag);
            break;

        case MB2_TAG_MEMORY_MAP:
            err = ParseMemoryMap(ka, tag);
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

    Debug::PrintF("MB2: Detected %d total tags.\r\n", i);

    return err;
}

/********************************************************************************************************************/
