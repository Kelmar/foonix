/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_KALLOC_SMALL_BUCKET_H__
#define __FOONIX_KERNEL_KALLOC_SMALL_BUCKET_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <type_traits>

#include <kernel/thread/spinlock.h>
#include <kernel/utils/align.h>
#include <kernel/utils/list.h>

#include <kernel/kalloc/small_slab.h>
#include <kernel/kalloc/hash.h>

/********************************************************************************************************************/

class SmallItemBucket
{
private:
    static constexpr const size_t INVALID_ITEM_INDEX = (size_t)-1;
    static constexpr const size_t START_ITEM_OFFSET = util::AlignCeiling<sizeof(uintptr_t)>(sizeof(SmallItemSlab));
    
    mutable thread::SpinLock m_mutex;

    uint8_t m_bucketIndex;
    size_t m_itemSize;
    size_t m_itemsPerSlab;

    List<SmallItemSlab> m_partial;
    List<SmallItemSlab> m_full;
    List<SmallItemSlab> m_empty;

private:
    /// @brief Find a SmallItemSlab with an available slot.
    SmallItemSlab *GetAvailableSlab();

    size_t GetPointerIndex(SmallItemSlab *slab, void *ptr) const;

public:
    constexpr SmallItemBucket(uint8_t bucketIndex) noexcept
        : m_mutex()
        , m_bucketIndex(bucketIndex)
        , m_itemSize(SmallItemHash::ItemSizeFromIndex(bucketIndex))
        , m_itemsPerSlab(SmallItemHash::ItemCountFromIndex(bucketIndex))
        , m_partial()
        , m_full()
        , m_empty()
    { }

    ~SmallItemBucket()
    { }

    bool IsEmpty() const
    {
        return m_partial.Count() == 0 && m_full.Count() == 0;
    }

    /// @brief Preallocate cnt slabs and places them into the bucket's empty list.
    void Prealloc(size_t cnt);

    /// @brief Perform garbage collection on this bucket.
    void GC();

    /// @brief Allocate an item from this bucket.
    void *Allocate();

    void Return(SmallItemSlab *slab, void *ptr);
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_KALLOC_SMALL_BUCKET_H__ */

/********************************************************************************************************************/
