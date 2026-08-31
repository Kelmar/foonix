/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILS_BITMAP_H__
#define __FOONIX_KERNEL_UTILS_BITMAP_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "cpu.h"

#include <kernel/math.h>

/********************************************************************************************************************/
/**
 * @brief Describes details for storing and working with bitmaps representing BITS number of bits.
 */
template <size_t BITS>
    requires (BITS > 0)
struct BitmapTraits
{
    using CellType = typename std::conditional_t<cpu::BitSize == 64, uint64_t, uint32_t>;

    /// @brief Number of bits this these traits are for.
    static constexpr const size_t BitCount = BITS;

    /// @brief Number of bits per "cell" (array item)
    static constexpr const size_t CellBits = sizeof(CellType) * 8;

    static constexpr const size_t CellBitShift = math::msb_index_v<(sizeof(CellType) * 8)>;

    /// @brief Number of left over bits that don't quite fill a full cell.
    static constexpr const size_t ExtraBitCount = BITS & (CellBits - 1);

    /// @brief Bit mask for testing bits in extra cell.
    static constexpr const CellType ExtraBitMask = static_cast<CellType>((1 << ExtraBitCount) - 1);

    /// @brief Count of array items that consume all bits of an uint32_t.
    static constexpr const size_t MinCellCount = BITS / CellBits;

    /// @brief Total size of array needed to hold all requested bits.
    static constexpr const size_t ArraySize = MinCellCount + (ExtraBitCount != 0 ? 1 : 0);

    /// @brief The size of the array in bytes
    static constexpr const size_t ArraySizeBytes = ArraySize * sizeof(CellType);

    /// @brief Get the array index needed to find the requested bit.
    static constexpr size_t GetCellIndex(size_t bitIndex)
    {
        return bitIndex / CellBits;
        //return bitIndex >> CellBitShift;
    }

    /// @brief Mask the index for the bit to test an individual cell.
    static constexpr CellType GetBit(size_t bitIndex)
    {
        return 1<< (bitIndex & (CellBits - 1));
    }

    /// @brief Invalid size indicator.
    static constexpr size_t npos = (size_t)(-1);
};

/********************************************************************************************************************/

namespace impl__
{
    // Specialized testers for testing blocks of bits.

    template <size_t BITS>
    struct BitBlockTest
    {
        typedef BitmapTraits<BITS> Traits;
        
        using CellType = Traits::CellType;

        static constexpr CellType AllBits = ((CellType)(0) - 1);

        static constexpr size_t BitCount = Traits::BitCount;
        static constexpr size_t CellBits = Traits::CellBits;
        static constexpr size_t npos = Traits::npos;

        static bool AllClear(const CellType *array)
        {
            for (size_t i = 0; i < Traits::MinCellCount; ++i)
            {
                if (array[i] != 0)
                    return false;
            }

            if (Traits::MinCellCount != Traits::ArraySize)
                return (array[Traits::MinCellCount] & Traits::ExtraBitCount) == 0;

            return true;
        }

        static bool AllSet(const CellType *array)
        {
            for (size_t i = 0; i < Traits::MinCellCount; ++i)
            {
                if (array[i] != AllBits)
                    return false;
            }

            if (Traits::MinCellCount != Traits::ArraySize)
                return (array[Traits::MinCellCount] & Traits::ExtraBitCount) == Traits::ExtraBitCount;

            return true;
        }

        static size_t FirstSet(const CellType *array)
        {
            for (size_t i = 0; i < Traits::MinCellCount; ++i)
            {
                size_t pos = math::lsb_index(array[i]);

                if (pos != 0)
                    return (i * CellBits) + (pos - 1);
            }

            if (Traits::MinCellCount != Traits::ArraySize)
            {
                size_t pos = math::lsb_index(array[Traits::MinCellCount]);

                if (pos <= Traits::ExtraBitCount)
                    return (Traits::MinCellCount * CellBits) + (pos - 1);
            }

            return Traits::npos;
        }

        static size_t FirstClear(const CellType *array)
        {
            for (size_t i = 0; i < Traits::MinCellCount; ++i)
            {
                size_t pos = math::lsb_index(~array[i]);

                if (pos != 0)
                    return (i * CellBits) + (pos - 1);
            }

            if (Traits::MinCellCount != Traits::ArraySize)
            {
                size_t pos = math::lsb_index(~array[Traits::MinCellCount]);

                if (pos <= Traits::ExtraBitCount)
                    return (Traits::MinCellCount * CellBits) + (pos - 1);
            }

            return Traits::npos;
        }
    };

    template <>
    struct BitBlockTest<32>
    {
        typedef BitmapTraits<32> Traits;

        using CellType = Traits::CellType;

        static bool AllClear(const CellType *array) { return array[0] == 0; }

        static bool AllSet(const CellType *array) { return array[0] == UINT32_MAX; }

        static size_t FirstSet(const CellType *array)
        {
            return math::lsb_index(array[0]) - 1;
        }

        static size_t FirstClear(const CellType *array)
        {
            return math::lsb_index(~array[0]) - 1;
        }
    };

    template <size_t BITS>
        requires (BITS > 0 && BITS < 32)
    struct BitBlockTest<BITS>
    {
        typedef BitmapTraits<BITS> Traits;

        using CellType = Traits::CellType;

        static bool AllClear(const CellType *array) { return array[0] == 0; }

        static bool AllSet(const CellType *array)
        {
            return (array[0] & Traits::ExtraBitMask) == Traits::ExtraBitMask;
        }

        static size_t FirstSet(const CellType *array)
        {
            size_t pos = math::lsb_index(array[0]);
            return pos < BITS ? (pos - 1) : Traits::npos;
        }

        static size_t FirstClear(const CellType *array)
        {
            size_t pos = math::lsb_index(~array[0]);
            return pos < BITS ? (pos - 1) : Traits::npos;
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

public:
    using CellType = Traits::CellType;
    static constexpr size_t npos = Traits::npos;

private:
    static constexpr const size_t ArraySize = Traits::ArraySize;
    static constexpr const size_t ArraySizeBytes = Traits::ArraySizeBytes;

    CellType m_items[ArraySize];

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

    /// @brief Return the index of the first set bit, or npos if all bits clear.
    size_t FirstSet() const { return BlockTest::FirstSet(m_items); }

    /// @brief Return the index of the first unset bit, or npos if all bits set.
    size_t FirstClear() const { return BlockTest::FirstClear(m_items); }

    #define BIT_OPERATION(NAME__, OP__)                    \
        void NAME__(size_t index) {                        \
            if (index >= BitCount) return;                 \
            size_t cellIdx = Traits::GetCellIndex(index);  \
            CellType bit = Traits::GetBit(index);          \
            m_items[cellIdx] = m_items[cellIdx] OP__ bit; }

    BIT_OPERATION(Set, |)
    BIT_OPERATION(Clear, & ~)
    BIT_OPERATION(Flip, ^)

    bool operator[](size_t index) const
    {
        if (index >= BitCount)
            return false;

        size_t itemIndex = Traits::GetCellIndex(index);
        CellType itemBit = Traits::GetBit(index);

        return (m_items[itemIndex] & itemBit) != 0;
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILS_BITMAP_H__ */

/********************************************************************************************************************/
