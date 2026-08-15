/********************************************************************************************************************/
/********************************************************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/kernel.h>

#include <kernel/allocator.h>
#include <kernel/debug.h>
#include <kernel/kernel_args.h>

#include <kernel/arch.h>
#include <kernel/vm.h>

//#include <kernel/page.h>

#include "cpu.h"

/********************************************************************************************************************/

PageBlock NullBlock(0, 0);

using namespace vmm;

/********************************************************************************************************************/

#if 0

namespace
{
    // For booting we just use a simple singly linked list of pages.
    struct BootPage
    {
        BootPage *next;
    };

    BootPage *bootPages = nullptr;
    size_t bootPageCount = 0;

    void InitBootPages()
    {
        for (size_t i = 0; i < g_kernelArguments.MemoryMapEntries; ++i)
        {
            Debug::PrintF("Mapping entries for group: %d\r\n", i);

            MemoryRange &memoryRange = g_kernelArguments.MemoryMap[i];

            for (size_t offset = 0; offset < memoryRange.Length; offset += PAGE_SIZE)
            {
                paddr_t addr = memoryRange.Base + offset;

                if (addr == 0x00107000)
                {
                    Debug::PrintF("Here's the spot!\r\n");
                }

                Debug::PrintF("Initializing page at 0x%08X\r\n", addr);

                BootPage *page = reinterpret_cast<BootPage *>(addr);

                Debug::PrintF("Setting page next\r\n");
                page->next = bootPages;

                Debug::PrintF("Setting bootPages\r\n");
                bootPages = page;

                Debug::PrintF("Adding to page count\r\n");
                ++bootPageCount;
            }
        }
    }
}

#endif

/********************************************************************************************************************/

void vmm::Init()
{
    g_kernelArguments.ShowAvailableMemory();

    //InitBootPages();

    //if (bootPageCount == 0)
    //    kpanic("Unable to allocate any boot pages\r\n");

    //Debug::PrintF("%d boot page(s) free.\r\n", bootPageCount);
}

/********************************************************************************************************************/
