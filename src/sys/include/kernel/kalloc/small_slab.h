/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_KALLOC_SMALL_SLAB_H__
#define __FOONIX_KERNEL_KALLOC_SMALL_SLAB_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/utils/bitmap.h>

/********************************************************************************************************************/

/**
 * @brief Slab for holding small allocations.
 *
 * @details
 * This class holds details for small allocations or less.  That is items who's size is 256 bytes or smaller.  The idea
 * behind this class is that the free/allocated bitmap is contained in the header of the slab, which does remove some
 * of the available space in a page, but tries to keep things fairly compact.
 *
 * At about 256 bytes though, we can only get 16 items in a 4096 page (without headers); thus this class starts to become
 * a hindrance instead of a help.
 */
class SmallItemSlab
{
private: 
    uint8_t m_bucket;         // Bucket to which this slab currently belongs
    uint16_t m_itemSize;      // The size of each slot (item) in bytes.
    uint16_t m_freeCount;     // The number of available items in this slab.
    uint8_t m_checksum;       // Also for alignment (pedantic check)

    Bitmap<256> m_freeBits;   // Bitmap of allocation marks.

#if MALLOC_CHECK_LEVEL >= 4
    static uint8_t CalcChecksum(const SmallItemSlab *slab);
    static bool ValidateSum(SmallItemSlab *slab);
#endif

public:
    // These are modified by List
    SmallItemSlab *Prev; // Previous slab in list.
    SmallItemSlab *Next; // Next slab in list.

    constexpr SmallItemSlab() noexcept
        : m_bucket(0xFF)
        , m_itemSize(0)
        , m_freeCount(0)
        , m_checksum(0)
        , m_freeBits()
        , Prev(nullptr)
        , Next(nullptr)
    { }

    ~SmallItemSlab() { }

    void Reset()
    {
        /*
         * For completeness and to ensure additional guards against errant free() calls, we set all
         * of the values in the slab to invalid values when it is moved into an empty list.
         */

        m_bucket = 0xFF;
        m_itemSize = 0;
        m_freeCount = 0;
        m_checksum = 0;
    }

    void Setup(uint8_t bucket, uint16_t itemSize, uint16_t freeCount)
    {
        m_bucket = bucket;
        m_itemSize = itemSize;
        m_freeCount = freeCount;

        UpdateSum();
    }

    inline uint8_t getBucket() const { return m_bucket; }
    inline size_t getFreeCount() const { return m_freeCount; }

    inline void UpdateSum()
    {
#if MALLOC_CHECK_LEVEL >= 4
        m_checksum = CalcChecksum(this);
#endif
    }

    inline size_t FirstFree() const
    {
        return m_freeBits.FirstClear();
    }

    inline bool IsMarked(size_t idx)
    {
        return m_freeBits[idx];
    }

    inline void Clear(size_t idx)
    {
        m_freeBits.Clear(idx);
        ++m_freeCount;
        //UpdateSum();
    }

    void Mark(size_t idx)
    {
        m_freeBits.Set(idx);
        --m_freeCount;
        //UpdateSum();
    }

    uintptr_t ItemStart() const
    {
        return reinterpret_cast<uintptr_t>(&this[1]);
    }

    static SmallItemSlab *GetSlabFromPtr(void *ptr);
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_KALLOC_SMALL_SLAB_H__ */

/********************************************************************************************************************/