/********************************************************************************************************************/
/********************************************************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/kernel.h>

#include <kernel/debug.h>
#include <kernel/boot_args.h>
#include <kernel/console.h>

#include <kernel/arch.h>

#include <kernel/vm.h>
#include <kernel/vm/page_allocator.h>
#include <kernel/vm/vpage_map.h>

#include <kernel/kalloc.h>

#include "cpu.h"
#include "paging.h"

/********************************************************************************************************************/

extern size_t kallocAllocatedPages;

namespace kernel
{
    memory::VPageMap *page_map;
}

using namespace vmm;

/********************************************************************************************************************/

static
void init_vpages()
{
#if 0
    auto &args = boot::Arguments::Instance();

    memory::VPageMapBuilder mapConfig {};

    mapConfig
        .CodeStart(args.KernelRange().BaseAligned())
        .HeapStart(args.HeapVirtual().BaseAligned())
    ;

    // Give architecture specific paging a chance to setup how the kernel::page_map is setup.
    paging::Init(mapConfig);

    auto result = mapConfig.BootBuild();

    if (result)
        kernel::page_map = result.value();
    else
    {
        err = result.error();
        Debug::PrintF("Error %d creating kernel virtual page allocator!", err);
        kpanic("Error creating kernel virtual page allocator!");
    }
#endif
}

/********************************************************************************************************************/

void vmm::Init()
{
    auto &args = boot::Arguments::Instance();

    // Remove any memory used by boot loader (e.g. Kernel code space)
    args.KnockoutUsedMemory();

    new (&page_allocator) paging::PageAllocator();

    init_vpages();

    args.CanAllocPages(true);
    args.ShowAvailableMemory();

    //paging::Init(kernVMap);
}

/********************************************************************************************************************/

void vmm::MemInfoCommand(size_t, const std::string_view[])
{
    console
        << "Boot Memory\r\n"
        << "    Start      Length\r\n";

    for (auto &mem : boot::Arguments::Instance().MemoryMap())
        console << "    0x" << hex(mem.Start, -8) << " 0x" << hex(mem.Length, -8) << "\r\n";

    int freePageCount = page_allocator.GetFreePages();

    size_t freemem = (freePageCount * cpu::PageSize) / 1024;

    console
        << "\r\nFree Pages: " << freePageCount << "\r\n"
        << "Free Memory: " << freemem << " KB\r\n";

    console
        << "\r\nkalloc stats\r\n"
        << "  pages allocated: " << kallocAllocatedPages << "\r\n";
}

/********************************************************************************************************************/
