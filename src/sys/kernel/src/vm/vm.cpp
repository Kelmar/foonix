/********************************************************************************************************************/
/********************************************************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/kernel.h>

#include <kernel/debug.h>
#include <kernel/kernel_args.h>
#include <kernel/console.h>

#include <kernel/arch.h>

#include <kernel/vm.h>
#include <kernel/vm/new.h>
#include <kernel/vm/page_allocator.h>

#include "cpu.h"

/********************************************************************************************************************/

using namespace vmm;

/********************************************************************************************************************/

void vmm::Init()
{
    g_kernelArguments.ShowAvailableMemory();

    new (&page_allocator) paging::PageAllocator();
}

/********************************************************************************************************************/

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

    int freePageCount = page_allocator.GetFreePages();

    size_t freemem = (freePageCount * cpu::PageSize) / 1024;

    console
        << "\r\nFree Pages: " << freePageCount << "\r\n"
        << "Free Memory: " << freemem << " KB\r\n";
}

/********************************************************************************************************************/
