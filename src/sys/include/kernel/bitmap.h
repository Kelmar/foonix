/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_BITMAP_H__
#define __FOONIX_KERNEL_BITMAP_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>
#include <stdint.h>

/********************************************************************************************************************/
/**
 * @brief Describes details for storing and working with bitmaps representing BITS number of bits.
 */
template <size_t BITS>
    requires (BITS > 0)
struct BitmapTraits
{
    /// @brief Number of bits this these traits are for.
    static constexpr const size_t BitCount = BITS;

    /// @brief Number of bits per "cell" (array item)
    static constexpr const size_t CellBits = sizeof(uint32_t) * 8;

    /// @brief Number of left over bits that don't quite fill a full cell.
    static constexpr const size_t ExtraBitCount = BITS & (CellBits - 1);

    /// @brief Bit mask for testing bits in extra cell.
    static constexpr const uint32_t ExtraBitMask = static_cast<uint32_t>((1 << ExtraBitCount) - 1);

    /// @brief Count of array items that consume all bits of an uint32_t.
    static constexpr const size_t MinCellCount = BITS / CellBits;

    /// @brief Total size of array needed to hold all requested bits.
    static constexpr const size_t ArraySize = MinCellCount + (ExtraBitCount != 0 ? 1 : 0);

    /// @brief The size of the array in bytes
    static constexpr const size_t ArraySizeBytes = ArraySize * sizeof(uint32_t);

    /// @brief Get the array index needed to find the requested bit.
    static constexpr int GetCellIndex(size_t bitIndex) { return bitIndex / CellBits; }

    /// @brief Mask the index for the bit to test an individual cell.
    static constexpr int GetBit(size_t bitIndex) { return bitIndex % CellBits; }
};

/********************************************************************************************************************/

namespace impl__
{
    // Specialized testers for testing blocks of bits.

    template <size_t BITS>
    struct BitBlockTest
    {
        typedef BitmapTraits<BITS> Traits;

        static bool AllClear(uint32_t *array)
        {
            for (int i = 0; i < Traits::MinCellCount; ++i)
            {
                if (array[i] != 0)
                    return false;
            }

            if (Traits::MinCellCount != Traits::ArraySize)
                return (array[Traits::MinCellCount] & Traits::ExtraBitCount) == 0;

            return true;
        }

        static bool AllSet(uint32_t *array)
        {
            for (int i = 0; i < Traits::MinCellCount; ++i)
            {
                if (array[i] != UINT32_MAX)
                    return false;
            }

            if (Traits::MinCellCount != Traits::ArraySize)
                return (array[Traits::MinCellCount] & Traits::ExtraBitCount) == Traits::ExtraBitCount;

            return true;
        }
    };

    template <>
    struct BitBlockTest<32>
    {
        typedef BitmapTraits<32> Traits;

        static bool AllClear(uint32_t *array) { return array[0] == 0; }

        static bool AllSet(uint32_t *array) { return array[0] == UINT32_MAX; }
    };

    template <size_t BITS>
        requires (BITS > 0 && BITS < 32)
    struct BitBlockTest<BITS>
    {
        typedef BitmapTraits<BITS> Traits;

        static bool AllClear(uint32_t *array) { return array[0] == 0; }

        static bool AllSet(uint32_t *array)
        {
            return (array[0] & Traits::ExtraBitMask) == Traits::ExtraBitMask;
        }
    };
}

/********************************************************************************************************************/
/**
 * @brief A fixed sized container containing BITS number of bits that maybe set or cleared.
 */
template <size_t BITS>
class Bitmap
{
private:
    using Traits = BitmapTraits<BITS>;
    using BlockTest = impl__::BitBlockTest<BITS>;

    static constexpr const size_t ArraySize = Traits::ArraySize;
    static constexpr const size_t ArraySizeBytes = Traits::ArraySizeBytes;

    static const size_t ItemBits = sizeof(uint32_t) * 8;

    uint32_t m_items[ArraySize];

    /* constructor */ Bitmap(const Bitmap &) = delete;
    /* constructor */ Bitmap(Bitmap &&) = delete;

public:
    static constexpr const size_t BitCount = Traits::BitCount;

    constexpr Bitmap(void)
    {
        memset(m_items, 0, ArraySizeBytes);
    }

    /// @brief Check if all bits are clear.
    bool Empty() const { return BlockTest::AllClear(m_items); }

    /// @brief Check of all bits are set.
    bool Full() const { return BlockTest::AllSet(m_items); }

    #define BIT_OPERATION(NAME__, OP__)             \
        void NAME__(size_t index) {                 \
            if (index >= BitCount) return;          \
            int idx = Traits::GetCellIndex(index);  \
            int bit = Traits::GetBit(index);        \
            m_items[idx] = m_items[idx] OP__ bit; }

    BIT_OPERATION(Set, |)
    BIT_OPERATION(Clear, & ~)
    BIT_OPERATION(Flip, ^)

    bool operator[](size_t index) const
    {
        if (index >= BitCount)
            return false;

        int itemIndex = Traits::GetCellIndex(index);
        int itemBit = Traits::GetBit(index);

        return (m_items[itemIndex] & itemBit) != 0;
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_BITMAP_H__ */

/********************************************************************************************************************/
