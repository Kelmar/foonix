/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_BITMAP_H__
#define __FOONIX_KERNEL_BITMAP_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/
/*
 * @brief A fixed sized container containing BITS number of bits that maybe set or cleared.
 */
template <size_t BITS>
class Bitmap
{
private:
    static const size_t ItemBits = sizeof(uint32_t) * 8;
    static const size_t ArrayCount = BITS / ItemBits;
    static const size_t ArraySize = ArrayCount * sizeof(uint32_t);

    uint32_t m_items[ArrayCount];

    /* constructor */ Bitmap(const Bitmap &) = delete;
    /* constructor */ Bitmap(Bitmap &&) = delete;

    constexpr int GetItem(size_t index) const
    {
        return index / ItemBits;
    }

    constexpr int GetBit(size_t index) const
    {
        return 1 << (index % ItemBits);
    }

public:
    static const size_t BitCount = BITS;

    /* constructor */ Bitmap(void)
    {
        memset(m_items, 0, ArraySize);
    }

    void Set(size_t index)
    {
        if (index >= BitCount)
            return;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        m_items[itemIndex] |= itemBit;
    }

    void Clear(size_t index)
    {
        if (index >= BitCount)
            return;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        m_items[itemIndex] &= ~itemBit;
    }

    bool operator[](size_t index) const
    {
        if (index >= BitCount)
            return false;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        return (m_items[itemIndex] & itemBit) != 0;
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_BITMAP_H__ */

/********************************************************************************************************************/
