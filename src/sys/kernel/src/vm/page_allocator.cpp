/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>

#include <kernel/kernel_args.h>

#include <kernel/debug.h>

#include <kernel/thread/lockguard.h>

#include <kernel/vm/page_allocator.h>

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
    , m_pageCache(nullptr)
    , m_pageCacheCount(0)
{
    InitBootPages();

    if (m_pageCacheCount == 0)
        kpanic("Unable to allocate any boot pages\r\n");

    g_kernelArguments.CanAllocPages = true;
}

/********************************************************************************************************************/

size_t paging::PageAllocator::GetFreePages() const
{
    thread::LockGuard lock(m_allocLock);

    // TODO: This should include pages that aren't in the cache.

    return m_pageCacheCount;
}

/********************************************************************************************************************/

size_t paging::PageAllocator::GetCacheSize() const
{
    thread::LockGuard lock(m_allocLock);
    return m_pageCacheCount;
}

/********************************************************************************************************************/

void paging::PageAllocator::InitBootPages()
{
    thread::LockGuard lock(m_allocLock);

    for (size_t i = 0; i < g_kernelArguments.MemoryMapEntries; ++i)
    {
        MemoryRange &memoryRange = g_kernelArguments.MemoryMap[i];

        for (size_t offset = 0; offset < memoryRange.Length; offset += cpu::PageSize)
        {
            paddr_t addr = memoryRange.Base + offset;

            if (addr < g_kernelArguments.HeapNext)
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

    page->next = m_pageCache;
    m_pageCache = page;

    ++m_pageCacheCount;
}

/********************************************************************************************************************/

paddr_t paging::PageAllocator::GetCachedPage()
{
    PageNode *result = m_pageCache;
    
    if (result != nullptr)
    {
        m_pageCache = result->next;
        --m_pageCacheCount;
        result->next = nullptr;
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
