/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_KALLOC_HASH_H__
#define __FOONIX_KERNEL_KALLOC_HASH_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <algorithm>

#include <kernel/kalloc/small_slab.h>

/********************************************************************************************************************/

/**
 * @brief Computes values based on sizes for small items.
 *
 * This effectively has two hash functions, after 256 bytes, we switch to a hash that places more of the larger
 * items into the same groups.  This may increase the internal fragmentation of the items in memory, but it also
 * means there are fewer buckets to manage.
 *
 * Should yield about 29 total buckets for systems with 4K pages.
 */
namespace SmallItemHash
{
    namespace impl__
    {
        static constexpr const size_t MAX_PAGE_SPACE = (cpu::PageSize - sizeof(SmallItemSlab));
        static constexpr const size_t MAX_ITEM_SIZE = MAX_PAGE_SPACE / 2;

        // Hash for numbers <= 256
        static constexpr size_t HashSizeSmall(size_t sz)
        {
            return (((sz + 15) >> 4) - 1);
        }

        // Hash for number > 256
        static constexpr size_t HashSizeMed(size_t sz)
        {
            return (((sz + 127) >> 7) - 3);
        }

        // Compute the size of the largest item for a small bucket given it's index.
        static constexpr size_t IndexSizeSmall(uint8_t index)
        {
            return static_cast<size_t>(index + 1) << 4;
        }

        // Compute the size of the largest item for a medium bucket given it's index.
        static constexpr size_t IndexSizeMed(uint8_t index)
        {
            return std::min(MAX_ITEM_SIZE, static_cast<size_t>(index + 3) << 7);
        }
    }

    // This will be different based on the size of the system's pointer.
    static constexpr const size_t MAX_ITEM_SIZE = impl__::MAX_ITEM_SIZE;

    static constexpr size_t IndexFromSize(size_t sz)
    {
        return sz <= 256 ? impl__::HashSizeSmall(sz) : (impl__::HashSizeMed(sz) + 16);
    }

    static constexpr size_t ItemSizeFromIndex(uint8_t bucket)
    {
        return bucket < 16 ? impl__::IndexSizeSmall(bucket) : impl__::IndexSizeMed(bucket - 16);
    }

    static constexpr size_t ItemCountFromIndex(uint8_t bucket)
    {
        return impl__::MAX_PAGE_SPACE / ItemSizeFromIndex(bucket);
    }

    static constexpr const size_t MAX_BUCKET_INDEX = IndexFromSize(MAX_ITEM_SIZE);
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_KALLOC_HASH_H__ */

/********************************************************************************************************************/
