/********************************************************************************************************************/
/*
 */
/********************************************************************************************************************/

#include <type_traits>
#include <concepts>

#include <kernel/utils/bitmap.h>
#include <kernel/utils/list.h>
#include <kernel/utils/span.h>

#include <kernel/debug.h>

#include <kernel/thread/lockguard.h>

#include <kernel/kalloc.h>

//#include "kalloc_config.h"
#define EMPTY_MAX 4

#include <kernel/kalloc/small_slab.h>
#include <kernel/kalloc/small_bucket.h>

#include <kernel/vm/page_allocator.h>

/********************************************************************************************************************/

extern size_t kallocAllocatedPages;

template <typename T>
static T *AllocatePageObj()
{
    void *ptr = page_allocator.AllocatePageAs<void *>();
    return new (ptr) T();
}

/********************************************************************************************************************/

SmallItemSlab *SmallItemBucket::GetAvailableSlab()
{
    SmallItemSlab *slab = m_partial.Front();

    if (slab)
        return slab;

    slab = m_empty.PopFront();

    if (!slab)
    {
        slab = AllocatePageObj<SmallItemSlab>();
        ++kallocAllocatedPages;
        //Debug::PrintF("Allocated page for %u bucket.\r\n", m_itemSize);
    }

    slab->Setup(
        m_bucketIndex,
        static_cast<uint16_t>(m_itemSize),
        static_cast<uint16_t>(m_itemsPerSlab));

    m_partial.PushFront(slab);
    return slab;
}

/********************************************************************************************************************/

size_t SmallItemBucket::GetPointerIndex(SmallItemSlab *slab, void *ptr) const
{
    size_t addr = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(&slab[1]);

    if ((addr % m_itemSize) != 0)
        return INVALID_ITEM_INDEX;

    size_t result = addr / m_itemSize;

    if (result >= m_itemsPerSlab)
        return INVALID_ITEM_INDEX;

    return result;
}

/********************************************************************************************************************/

#if 0
void SmallItemBucket::Prealloc(size_t cnt)
{
    // Preload some slabs into the empty slab list.
    for (size_t i = 0; i < cnt; ++i)
    {
        SmallItemSlab *slab = AllocatePageObj<SmallItemSlab>();
        ++kallocAllocatedPages;
        //Debug::PrintF("Allocated page for %u bucket.\r\n", m_itemSize);
        m_empty.PushBack(slab);
    }
}
#endif

/********************************************************************************************************************/

void SmallItemBucket::GC()
{
    thread::LockGuard l(m_mutex);

    while (m_empty.Count() > EMPTY_MAX)
    {
        auto slab = m_empty.PopBack();
        slab->~SmallItemSlab();
        page_allocator.ReleasePage(slab);
        --kallocAllocatedPages;
        //Debug::PrintF("Released page for %u bucket.\r\n", m_itemSize);
    }
}

/********************************************************************************************************************/

void *SmallItemBucket::Allocate()
{
    thread::LockGuard l(m_mutex);

    SmallItemSlab *slab = GetAvailableSlab();

    if (!slab)
        return nullptr; // Out of pages (and probably memory)

    size_t idx = slab->FirstFree();

#if MALLOC_CHECK_LEVEL >= 2
    if (idx == Bitmap<256>::npos || (idx >= m_itemsPerSlab))
        CHECK_FAIL(); // Shouldn't happen.
#endif

    slab->Mark(idx);

    if (!slab->getFreeCount())
    {
        m_partial.Remove(slab);
        m_full.PushFront(slab);
    }

    uintptr_t addr = slab->ItemStart() + (idx * m_itemSize);
    return reinterpret_cast<void *>(addr);
}

/********************************************************************************************************************/

void SmallItemBucket::Return(SmallItemSlab *slab, void *ptr)
{
    thread::LockGuard l(m_mutex);

    size_t idx = GetPointerIndex(slab, ptr);

    if (idx == INVALID_ITEM_INDEX)
    {
#if MALLOC_CHECK_LEVEL >= 1
        CHECK_FAIL();
#else
        return;
#endif
    }

    if (slab->IsMarked(idx))
    {
        size_t lastCount = slab->getFreeCount();

        slab->Clear(idx);

        if (lastCount == 0)
        {
            m_full.Remove(slab);
            m_partial.PushBack(slab);
        }
        else if (lastCount == (m_itemsPerSlab - 1))
        {
            slab->Reset();

            m_partial.Remove(slab);
            m_empty.PushBack(slab); // Might need to sort these pages as we release them.
        }
    }
}

/********************************************************************************************************************/
