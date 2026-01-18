/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_BITMAP_H__
#define __FOONIX_KERNEL_BITMAP_H__

/*************************************************************************/

#include <stddef.h>
#include <string.h>

/*************************************************************************/

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

        int itemIndex = index / ArrayCount;
        int itemBit = 1 << (index % ArrayCount);

        m_items[itemIndex] |= itemBit;
    }

    void Clear(size_t index)
    {
        if (index > BitCount)
            return;

        int itemIndex = index / ArrayCount;
        int itemBit = 1 << (index % ArrayCount);

        m_items[itemIndex] &= ~itemBit;
    }

    bool operator[](size_t index) const
    {
        if (index > BitCount)
            return false;

        int itemIndex = index / ArrayCount;
        int itemBit = 1 << (index % ArrayCount);

        return (m_items[itemIndex] & itemBit) != 0;
    }
};

/*************************************************************************/

#endif /* __FOONIX_KERNEL_BITMAP_H__ */

/*************************************************************************/
