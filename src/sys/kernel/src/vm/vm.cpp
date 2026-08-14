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

namespace
{
#if 0
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
            MemoryRange &memoryRange = g_kernelArguments.MemoryMap[i];

            for (size_t offset = 0; offset < memoryRange.Length; offset += PAGE_SIZE)
            {
                BootPage *page = reinterpret_cast<BootPage *>(memoryRange.Base + offset);
                page->next = bootPages;

                bootPages = page;
                ++bootPageCount;
            }
        }
    }
#endif
}

/********************************************************************************************************************/

void vmm::Init()
{
    g_kernelArguments.ShowAvailableMemory();

    /*
    InitBootPages();

    if (bootPageCount == 0)
        kpanic("Unable to allocate any boot pages\r\n");

    Debug::PrintF("%d boot page(s) free.\r\n", bootPageCount);
    */
}

/********************************************************************************************************************/
