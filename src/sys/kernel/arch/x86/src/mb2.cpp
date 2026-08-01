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

#include "cpu.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "bootinfo.h"

/********************************************************************************************************************/

int MB2::ReadInfo(KernelArgs *ka, uint32_t multiboot_ptr)
{
    Debug::PrintF("Multiboot 2 load detected.\r\n");

    Debug::PrintF("Multiboot: 0x%08X\r\n", multiboot_ptr);

    auto info = reinterpret_cast<mb2_info *>(multiboot_ptr);

    Debug::PrintF("MB2 Info Size: %d\r\n", info->total_size);

    uintptr_t ptr = multiboot_ptr + sizeof(mb2_info);

    size_t sz = sizeof(mb2_info);
    int i = 0;
    
    while (sz < info->total_size)
    {
        // MB2 tags should be aligned on 8-byte boundaries.
        uintptr_t p2 = util::AlignCeiling<8>(ptr);
        size_t diff = p2 - ptr;

        sz += diff;
        ptr = p2;

        auto tag = reinterpret_cast<mb2_tag *>(ptr);
        Debug::PrintF("Tag %d: T(%d) SZ(%d)\r\n", i, tag->type, tag->size);

        sz += tag->size;
        ptr += tag->size;

        ++i;
    }

    Debug::PrintF("MB2: Detected %d total tags.\r\n", i);

    return 0; //Kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/
