/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kassert.h>

#include <kernel/kernel.h>
#include <kernel/boot_args.h>
#include <kernel/debug.h>
#include <kernel/thread/lockguard.h>

#include <kernel/vm/page_allocator.h>
#include <kernel/vm/boot_allocator.h>

using namespace memory;

/********************************************************************************************************************/

BootPageAllocator::BootPageAllocator(boot::ArgumentData *data) noexcept
    : m_data(data)
    , m_totalPages(0)
    , m_free(nullptr)
    , m_freeCount(0)
{
    GetPagesFromMappings();
    
    if (!m_totalPages)
        m_totalPages = m_data->HighMemorySizeKByte / (cpu::PageSize / 1024); // Okay, guess based on most basic of info...

    ASSERT(m_totalPages > 0, "Can't figure out number of pages available for allocations on boot!");
}

/********************************************************************************************************************/

void BootPageAllocator::GetPagesFromMappings()
{
    // Try for a MemoryMap to determine number of available pages.
    for (size_t i = 0; i < m_data->MemoryMapCount; ++i)
    {
        boot::MemoryMapping &mapping = m_data->MemoryMap[i];

        if (mapping.Type != boot::MemoryType::Available)
            continue;

        paddr_t mapEnd = mapping.Start + mapping.Length;

        if ((mapping.Start <= m_data->HeapStart) && (m_data->HeapNext < mapEnd))
        {
            // Found it....
            mapEnd = paging::AlignFloor(mapEnd);
            m_totalPages = (mapEnd - mapping.Start) / cpu::PageSize;
            break;
        }
    }
}

/********************************************************************************************************************/

size_t BootPageAllocator::GetFreePages() const
{
    size_t allocated = (m_data->HeapNext - m_data->HeapStart) / cpu::PageSize;

    return (m_totalPages - allocated) + m_freeCount;
}

/********************************************************************************************************************/

size_t BootPageAllocator::AllocatePages(size_t count, PagePointer pointers[])
{
    if (count > GetFreePages())
        return 0;

    /*
     * Note that we aren't attempting to figure out buddy splits at this point; we just do a one to one 
     * allocation and let the buddy allocator sort out the damage later.
     */

    paddr_t pptr = m_data->HeapNext;
    vaddr_t vptr = m_data->VirtHeapNext;

    size_t result = 0;

    while (result < count)
    {
        if (m_free)
        {
            // Pull from free list first
            paddr_t p = reinterpret_cast<paddr_t>(m_free);
            pointers[result] = PagePointer::FromPhysical(p);
            m_free = m_free->Next;
            --m_freeCount;
        }
        else
        {
            pointers[result] = PagePointer::FromPhysical(pptr);
            pptr += cpu::PageSize;
            vptr += cpu::PageSize;
        }

        ++result;
    }

    m_data->HeapNext = pptr;
    m_data->VirtHeapNext = vptr;

    return result;
}

/********************************************************************************************************************/

void BootPageAllocator::ReleasePages(util::span<PagePointer> ptrs)
{
    for (auto &pagePtr : ptrs)
    {
        // We can only handle physical pages and pages of order 0 (any other order would have come from another allocator!)
        ASSERT(pagePtr.IsPhysical() && (pagePtr.Order() == 0), "Invalid PagePointer passed to BootAllocator::ReleasePages()");

        PageNode *node = pagePtr.PointerTo<PageNode *>();
        pagePtr.Clear(); // We are taking ownership of this page.

        node->Next = m_free;
        m_free = node;
        ++m_freeCount;
    }
}

/********************************************************************************************************************/
