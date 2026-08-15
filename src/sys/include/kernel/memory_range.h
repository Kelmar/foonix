/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_MEMORY_RANGE_H__
#define __FOONIX_MEMORY_RANGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <algorithm>
#include <utility>
#include <expected>

#include <kernel/kernel.h>
#include <kernel/vm.h>

/********************************************************************************************************************/
/**
 * @brief Error code as to why memory ranges might not be mergeable.
 *
 * For now just has the one value, here for easy expansion in the future.
 */
enum class MemoryMergeError
{
    NotContiguous
};

/********************************************************************************************************************/

/**
 * @brief Represents a contiguous range of memory.
 *
 * Holds a base address and a length.
 */
struct MemoryRange
{
    /*
     * I feel like what we need here is std::sized_range or some other structure from the 
     * C++ STL, but I'm unclear on how these are implemented, used, or their intent just yet.
     *
     * For now, we'll keep it simple.
     *
     *          -- B.Simonds (July 26, 2026)
     */

    uintptr_t Base;
    size_t Length;

    constexpr MemoryRange() noexcept
        : Base(0)
        , Length(0)
    {
    }

    constexpr MemoryRange(uintptr_t start, size_t length) noexcept
        : Base(start)
        , Length(length)
    {
    }

    MemoryRange(const MemoryRange &lsh) noexcept
        : Base(0)
        , Length(0)
    {
        operator =(lsh);
    }

    MemoryRange(MemoryRange &&rhs) noexcept
        : Base(0)
        , Length(0)
    {
        operator =(std::move(rhs));
    }

    /**
     * @brief Build a memory range that includes both start and end memory addresses.
     */
    static inline
    constexpr MemoryRange FromAddresses(uintptr_t start, uintptr_t end)
    {
        if (start > end)
            std::swap(start, end);
        
        size_t length = end - start + 1;
        return MemoryRange(start, length);
    }

    /**
     * @brief Get the base address rounded down to the nearest page boundary.
     */
    inline
    constexpr uintptr_t BaseAligned() const noexcept { return paging::AlignFloor(Base); }

    /**
     * @brief Calculate the address at the end of the range.
     *
     * Returns the last valid address contained within the range.
     */
    inline
    constexpr uintptr_t End() const noexcept { return Base + Length - 1; }

    /**
     * @brief Get the ending address of the memory range rounded up to the nearest page boundary minus one.
     *
     * Thus for a 4KB page size, if the Base is at 0x00100000, and Length is 300 (0x12C), the AlignedEnd()
     * would return 0x00100FFF, consistent with the behavior of End().
     */
    inline
    constexpr uintptr_t EndAligned() const noexcept { return paging::AlignCeiling(End()) - 1; }

    /// @brief Returns true if the range has a zero length.
    inline
    bool Empty() const noexcept { return Length == 0; }

    /// @brief Checks to see if the other range overlaps with this range.
    inline
    bool Overlaps(const MemoryRange &r) const noexcept
    {
        uintptr_t end = End();
        uintptr_t rend = r.End();

        return
            (Length != 0) && (r.Length != 0) &&
            (Base <= rend) && (end >= r.Base);
    }

    /// @brief Check to see if this range is next to another range.
    inline
    bool Contiguous(const MemoryRange &r) const noexcept
    {
        if ((Length == 0) || (r.Length == 0))
            return false;

        if (r == *this)
            return true;

        uintptr_t end = End();
        uintptr_t rend = r.End();

        if ((end + 1) >= r.Base && (end < rend))
            return true;

        return ((rend + 1) >= Base) && (rend < end);
    }

    inline
    MemoryRange &operator =(const MemoryRange &r) noexcept
    {
        Base = r.Base;
        Length = r.Length;

        return *this;
    }

    inline
    MemoryRange &operator =(MemoryRange &&r) noexcept
    {
        std::swap(Base, r.Base);
        std::swap(Length, r.Length);

        return *this;
    }

    /**
     * @brief Attempt to merge two MemoryRange objects into one.
     *
     * Returns the merged memory range if successful.  MemoryMergeError if not.
     */
    inline static 
    auto Merge(const MemoryRange &r1, const MemoryRange &r2) -> std::expected<MemoryRange, MemoryMergeError>
    {
        if (!r1.Contiguous(r2))
            return std::unexpected(MemoryMergeError::NotContiguous);

        uintptr_t minBase = std::min(r1.Base, r2.Base);
        uintptr_t maxEnd = std::max(r1.End(), r2.End());

        return MemoryRange(minBase, maxEnd - minBase + 1);
    }

    /// @brief Non-empty test operator.
    inline
    operator bool() const noexcept { return !Empty(); }

    inline
    bool operator ==(const MemoryRange &rhs) const noexcept = default;
};

/********************************************************************************************************************/

#endif /* __FOONIX_MEMORY_RANGE_H__ */

/********************************************************************************************************************/
