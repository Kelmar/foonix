/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_BITMAP_H__
#define __FOONIX_KERNEL_BITMAP_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>

/********************************************************************************************************************/

template <size_t bits>
class Bitmap
{
private:
    static const size_t ItemBits = sizeof(uint32_t) * 8;
    static const size_t ArrayCount = bits / ItemBits;
    static const size_t ArraySize = ArrayCount * sizeof(uint32_t);

    uint32_t m_items[ArrayCount];

    /* constructor */ Bitmap(const Bitmap &) = delete;
    /* constructor */ Bitmap(Bitmap &&) = delete;

    constexpr int GetItem(size_t index) const
    {
        return index / ArrayCount;
    }

    constexpr int GetBit(size_t index) const
    {
        return 1 << (index % ArrayCount);
    }

public:
    static const size_t BitCount = bits;

    /* constructor */ Bitmap(void)
    {
        memset(m_items, 0, ArraySize);
    }

    void Set(size_t index)
    {
        if (index > BitCount)
            return;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        m_items[itemIndex] |= itemBit;
    }

    void Clear(size_t index)
    {
        if (index > BitCount)
            return;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        m_items[itemIndex] &= ~itemBit;
    }

    bool operator[](size_t index) const
    {
        if (index > BitCount)
            return false;

        int itemIndex = GetItem(index);
        int itemBit = GetBit(index);

        return (m_items[itemIndex] & itemBit) != 0;
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_BITMAP_H__ */

/********************************************************************************************************************/
