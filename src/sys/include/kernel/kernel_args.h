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
#include <kernel/memory_range.h>
#include <kernel/span.h>

/********************************************************************************************************************/

/// @brief Startup state of various memory management components.
class MemManagerState
{
public:
    enum Values
    {
        /// @brief No memory management has been started yet.
        Uninitialized = 0,

        /// @brief Paging is being initialized.
        Initializing = 100,

        /// @brief General paging is initialized but malloc and friends aren't ready yet.
        Paging = 200,

        /// @brief Memory management is ready.
        Initialized = 300
    };

private:
    Values m_value;

public:
    constexpr MemManagerState() noexcept : m_value(Uninitialized) { }
    constexpr MemManagerState(Values value) noexcept : m_value(value) { }

    constexpr MemManagerState(const MemManagerState &rhs) noexcept = default;
    constexpr MemManagerState(MemManagerState &&rhs) noexcept = default;
    
    constexpr MemManagerState &operator = (const MemManagerState &rhs) noexcept = default;
    constexpr MemManagerState &operator = (MemManagerState &&) noexcept = default;

    constexpr MemManagerState &operator = (Values value) noexcept
    {
        m_value = value;
        return *this;
    }
    
    constexpr operator Values() const { return m_value; }

    /// @brief Check to see if a given operation is safe yet.
    constexpr bool IsSafe(Values value)
    {
        unsigned int v = static_cast<unsigned int>(value);
        unsigned int val = static_cast<unsigned int>(m_value);

        return (v / 100) >= (val / 100);
    }
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
    static const size_t MaxMemoryEntries = 16;

    /// @brief State of the memory management loading.
    MemManagerState MemManagerState;

    /// @brief Size of the memory in KBytes
    size_t MemorySizeKByte;

    /// @brief Physical memory location where the kernel is located.
    MemoryRange KernelCode;

    // Number of valid entries in the below array.
    size_t MemoryMapEntries;

    /// @brief Physical address of where the kernel heap starts.
    paddr_t HeapStart;

    /// @brief Physical address of the next available heep address is.
    paddr_t HeapNext;

    /**
     * @brief List of available memory.
     */
    MemoryRange MemoryMap[MaxMemoryEntries];

    constexpr KernelArgs()
        : MemManagerState()
        , MemorySizeKByte(0)
        , KernelCode()
        , MemoryMapEntries(0)
        , HeapStart(0)
        , HeapNext(0)
    {
    }

    void SetCommandLine(const util::span<char> &str)
    {
        SetCommandLine(str.data(), str.size_bytes());
    }

    void SetCommandLine(const char *data, size_t len);

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

extern KernelArgs g_kernelArguments;

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARGS_H__ */

/********************************************************************************************************************/
