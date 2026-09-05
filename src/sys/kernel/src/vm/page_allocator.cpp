/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>

#include <kernel/boot_args.h>

#include <kernel/debug.h>

#include <kernel/thread/lockguard.h>

#include <kernel/vm/page_allocator.h>

#include "paging.h"

/********************************************************************************************************************/

paging::PageAllocator page_allocator;

/*
 * On x64 we only map the first 2MB starting out; for x32, we're doing 4MB.  For now we just add the lesser of the
 * two into our page allocator list.
 */
//constexpr const size_t Mark4MB = 4 * (1 << 20);
constexpr const size_t Mark2MB = 2 * (1 << 20);

/********************************************************************************************************************/

using namespace paging;

paging::PageAllocator::PageAllocator() noexcept
    : m_allocLock()
    , m_pages(nullptr)
    , m_totalPages(0)
    , m_pagesAvailable(0)
    , m_pageCache(nullptr)
    , m_pageCacheCount(0)
{
    InitBootPages();

    if (m_pageCacheCount == 0)
        kpanic("Unable to allocate any boot pages\r\n");
}

/********************************************************************************************************************/

size_t paging::PageAllocator::GetFreePages() const
{
    thread::LockGuard lock(m_allocLock);
    return m_pagesAvailable;
}

/********************************************************************************************************************/

size_t paging::PageAllocator::GetCacheSize() const
{
    thread::LockGuard lock(m_allocLock);
    return m_pageCacheCount;
}

/********************************************************************************************************************/

void paging::PageAllocator::AllocateNodes()
{
    (void)m_pages;
    (void)m_totalPages;

    // Compute the available memory from the memory map minus whatever has already been allocated.
    size_t totalMemSize = 0;

    auto &args = boot::Arguments::Instance();

    paddr_t heapStart = args.HeapPhysical().BaseAligned();

    for (auto &mem : args.MemoryMap())
    {
        MemoryRange memoryRange = mem.ToRange();

        uintptr_t end = memoryRange.EndAligned();

        if (end < heapStart)
            continue;

        uintptr_t start = std::max(memoryRange.BaseAligned(), heapStart);

        totalMemSize += (end - start) + 1;
        break; // For now we can only handle the one section of memory.
    }

    m_totalPages = totalMemSize >> cpu::PageShift;

    Debug::PrintF("Staring Memory size: %u (%u)\r\n", totalMemSize, m_totalPages);

#if 0
    paddr_t ptr = args.HeapPhysical().EndAligned();
    vaddr_t virtPtr = args.HeapVirtual().EndAligned();
    
    m_pages = reinterpret_cast<PageNode *>(ptr);
    (void)m_pages;

    //vaddr_t heapPtr = 

    for (size_t i = 0; i < m_totalPages; ++i)
    {
        paging::g_bootPageTable.MapPage(ptr, virtPtr, PageFlags::Write | PageFlags::Kernel);
        ptr += cpu::PageSize;
        virtPtr += cpu::PageSize;
    }

    //kernel::arguments::HeapNext = ptr;
    //kernel::arguments::VirtHeapNext = virtPtr;
#endif
}

/********************************************************************************************************************/

void paging::PageAllocator::InitBootPages()
{
    thread::LockGuard lock(m_allocLock);

    AllocateNodes();

    auto &args = boot::Arguments::Instance();
    paddr_t heapEnd = args.HeapPhysical().End();

    for (auto &mapping : args.MemoryMap())
    {
        for (size_t offset = 0; offset < mapping.Length; offset += cpu::PageSize)
        {
            paddr_t addr = mapping.Start + offset;

            if (addr < heapEnd)
                continue; // Ignore pages before the kernel's heap next.

            if (addr >= Mark2MB)
                return; // We've reached past our 2MB identity map.

            AddPageToCache(addr);
        }
    }

    Debug::PrintF("%d boot page(s) free.\r\n", m_pageCacheCount);
}

/********************************************************************************************************************/

void paging::PageAllocator::AddPageToCache(paddr_t addr)
{
    PageNode *page = reinterpret_cast<PageNode *>(addr);

    page->Next = m_pageCache;
    m_pageCache = page;

    ++m_pageCacheCount;
}

/********************************************************************************************************************/

paddr_t paging::PageAllocator::GetCachedPage()
{
    PageNode *result = m_pageCache;
    
    if (result != nullptr)
    {
        m_pageCache = result->Next;
        --m_pageCacheCount;
        result->Next = nullptr;
    }

    return reinterpret_cast<paddr_t>(result);
}

/********************************************************************************************************************/

paddr_t paging::PageAllocator::AllocatePage()
{
    thread::LockGuard lock(m_allocLock);

    return GetCachedPage();
}

/********************************************************************************************************************/

void paging::PageAllocator::ReleasePage(paddr_t addr)
{
    thread::LockGuard lock(m_allocLock);

    AddPageToCache(addr);
}

/********************************************************************************************************************/
