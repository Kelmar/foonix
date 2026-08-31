/********************************************************************************************************************/

/********************************************************************************************************************/

#include <stdint.h>

#include "cpu.h"

#include <kernel/utils/bitmap.h>

//#include "dmalloc_config.h"

#include <kernel/kalloc/hash.h>
#include <kernel/kalloc/small_slab.h>

/********************************************************************************************************************/

#if MALLOC_CHECK_LEVEL >= 4
uint8_t SmallItemSlab::CalcChecksum(const SmallItemSlab *slab)
{
    // We are actually interested in the pointer value here, not what the pointer points to.
    uint8_t *ptrBytes = reinterpret_cast<uint8_t *>(&slab);

    const uint8_t *szBytes = reinterpret_cast<const uint8_t *>(&(slab->m_itemSize));

    /*
     * This is specific to 32-bit pointers, however even for 64-bit systems, really only care about the lowest significant
     * 4 bytes. The others aren't as likely to change.  Additionally one of these could be dropped as it'll always be zero.
     *
     * We need an cpu::endian flag for this function to be completely correct.
     */
    uint8_t sum = ptrBytes[0] + ptrBytes[1] + ptrBytes[2] + ptrBytes[3];
    sum += slab->m_bucket + szBytes[0] + szBytes[1];

    sum += 31; // Add a mersenne prime for good measure.

    return sum;
}

bool SmallItemSlab::ValidateSum(SmallItemSlab *slab)
{
    uint8_t sum = ~CalcChecksum(slab) + slab->m_checksum + 1;
    return sum == 0;
}
#endif

/********************************************************************************************************************/

SmallItemSlab *SmallItemSlab::GetSlabFromPtr(void *ptr)
{
    uintptr_t aligned = reinterpret_cast<uintptr_t>(ptr) & cpu::PageMask;
    SmallItemSlab *slab = reinterpret_cast<SmallItemSlab *>(aligned);

    if (slab->m_bucket > SmallItemHash::MAX_BUCKET_INDEX)
        return nullptr;

#if MALLOC_CHECK_LEVEL >= 1
    // Validate that m_bucket and m_itemSize seem related.
    uint16_t sz = SmallItemHash::ItemSizeFromIndex(slab->m_bucket);

    if (sz != slab->m_itemSize)
        return nullptr;
#endif

#if (MALLOC_CHECK_LEVEL >= 2) && (MALLOC_CHECK_LEVEL < 4)
    // At this level Checksum is never set to anything but zero; we can use that fact to check it.
    if (m_checksum != 0)
        return nullptr;
#endif

#if MALLOC_CHECK_LEVEL >= 4
    if (!ValidateSum(slab))
        return nullptr;
#endif

    return slab;
}

/********************************************************************************************************************/
