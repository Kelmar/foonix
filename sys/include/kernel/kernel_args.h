/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARGS_H__
#define __FOONIX_KERNEL_ARGS_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <algorithm>
#include <utility>
#include <expected>

#include <kernel/kernel.h>

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

    paddr_t Base;
    size_t Length;

    constexpr MemoryRange() noexcept
        : Base(0)
        , Length(0)
    {
    }

    constexpr MemoryRange(paddr_t start, size_t length) noexcept
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
     * @brief Calculate the address at the end of the range.
     *
     * Returns the last valid address contained within the range.
     */
    inline
    uint32_t End() const noexcept { return Base + Length - 1; }

    /// @brief Returns true if the range has a zero length.
    inline
    bool Empty() const noexcept { return Length == 0; }

    /// @brief Checks to see if the other range overlaps with this range.
    inline
    bool Overlaps(const MemoryRange &r) const noexcept
    {
        paddr_t end = End();
        paddr_t rend = r.End();

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

        paddr_t end = End();
        paddr_t rend = r.End();

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

        paddr_t minBase = std::min(r1.Base, r2.Base);
        paddr_t maxEnd = std::max(r1.End(), r2.End());

        return MemoryRange(minBase, maxEnd - minBase + 1);
    }

    /// @brief Non-empty test operator.
    inline
    operator bool() const noexcept { return !Empty(); }

    inline
    bool operator ==(const MemoryRange &rhs) const noexcept = default;
};

/********************************************************************************************************************/

class KernelArgs
{
private:
    /*
     * @brief Sort memory mappings so they appear in order.
     */
    void SortMappings();

    /// @brief Remove empty (zero length) mappings.
    void RemoveDeadMappings();

    /*
     * @brief Merge contiguous memory mappings into single maps.
     *
     * Attempt to crunch down memory map usage by merging contiguous memory map entries into larger single entries.
     */
    void MergeContiguousMappings();

    /*
     * @brief Slide MemoryMap entries down starting from start and continuing to the end.
     *
     * Reduces the length of the MemoryMap by exactly one.
     */ 
    void SlideEntries(int start);

public:
    static const size_t MAX_MEMORY_ENTRIES = 16;

    /// @brief Size of the memory in KBytes
    size_t MemorySizeKByte;

    /// @brief Memory location where the kernel is located.
    MemoryRange KernelCode;

    // Number of valid entries in the below array.
    size_t MemoryMapEntries;

    /**
     * @brief List of available memory.
     */
    MemoryRange MemoryMap[MAX_MEMORY_ENTRIES];

    constexpr KernelArgs()
        : MemorySizeKByte(0)
        , KernelCode()
        , MemoryMapEntries(0)
    {
    }

    /**
     * @brief Adds a potential range of free memory from a detected memory map.
     *
     * Does not attempt to resolve if memory is already used or not, simply adds it to
     * the initial bootup memory mapping provided by BIOS, Multiboot, hardware detection
     * or whatever.
     *
     * Note that this function may condense multiple mappings into one if they are
     * contiguous regions of memory.
     *
     * @returns True if the memory was successfully added to the map.  False if there
     * was not enough space in the map table to add the mapping.
     */
    bool AddMemoryMap(paddr_t base, size_t length);

    /**
     * @brief Removes used memory from boot memory map.
     *
     * Runs over the detected free memory and removes any memory used by kernel and drivers
     * loaded by multiboot, EFI or other boot loader modules that weren't detected/removed
     * from the call to the AddMemoryMap() function.
     */
    void KnockoutUsedMemory();

    /// @brief Display the list of available memory ranges.
    void ShowAvailableMemory(void);
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARGS_H__ */

/********************************************************************************************************************/
