/********************************************************************************************************************/
/********************************************************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/kernel.h>

#include <kernel/allocator.h>
#include <kernel/debug.h>
#include <kernel/kernel_args.h>
#include <kernel/console.h>

#include <kernel/arch.h>

#include <kernel/vm.h>
#include <kernel/vm/new.h>

//#include <kernel/page.h>

#include "cpu.h"

/********************************************************************************************************************/

PageBlock NullBlock(0, 0);

using namespace vmm;

/********************************************************************************************************************/

namespace
{
    // For booting we just use a simple singly linked list of pages.
    struct BootPage
    {
        BootPage *next;
    };

    BootPage *bootPages = nullptr;
    size_t bootPageCount = 0;

    constexpr const size_t Mark4MB = 4 * (1 << 20);

    void InitBootPages()
    {
        for (size_t i = 0; i < g_kernelArguments.MemoryMapEntries; ++i)
        {
            MemoryRange &memoryRange = g_kernelArguments.MemoryMap[i];

            for (size_t offset = 0; offset < memoryRange.Length; offset += cpu::PageSize)
            {
                paddr_t addr = memoryRange.Base + offset;

                if (addr < g_kernelArguments.HeapStart)
                    continue; // Ignore pages before the kernel's heap start.

                if (addr >= Mark4MB)
                    return; // We've reached past our 4MB identity map.

                BootPage *page = reinterpret_cast<BootPage *>(addr);
                page->next = bootPages;

                bootPages = page;
                ++bootPageCount;
            }
        }
    }
}

/********************************************************************************************************************/

void vmm::Init()
{
    g_kernelArguments.ShowAvailableMemory();

    InitBootPages();

    if (bootPageCount == 0)
        kpanic("Unable to allocate any boot pages\r\n");

    Debug::PrintF("%d boot page(s) free.\r\n", bootPageCount);
}

void vmm::MemInfoCommand(size_t, const std::string_view[])
{
    console
        << "Boot Memory\r\n"
        << "    Start      Length\r\n";

    for (uint32_t i = 0; i < g_kernelArguments.MemoryMapEntries; ++i)
    {
        const MemoryRange &mem = g_kernelArguments.MemoryMap[i];
        console << "    0x" << hex(mem.Base, -8) << " 0x" << hex(mem.Length, -8) << "\r\n";
    }

    size_t freemem = (bootPageCount * cpu::PageSize) / 1024;

    console
        << "\r\nFree Pages: " << bootPageCount << "\r\n"
        << "Free Memory: " << freemem << " KB\r\n";
}

/********************************************************************************************************************/
