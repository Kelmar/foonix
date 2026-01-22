/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_ARGS_H__
#define __FOONIX_KERNEL_ARGS_H__

/*************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <utility>

#include <kernel/kernel.h>

/*************************************************************************/

struct MemoryRange
{
    uint32_t Base;
    size_t Length;

    constexpr MemoryRange()
        : Base(0)
        , Length(0)
    {
    }

    inline
    uint32_t End() const { return Base + Length; }

    inline
    MemoryRange &operator =(const MemoryRange &r)
    {
        Base = r.Base;
        Length = r.Length;

        return *this;
    }

    inline
    MemoryRange &operator =(MemoryRange &&r)
    {
        std::swap(Base, r.Base);
        std::swap(Length, r.Length);

        return *this;
    }
};

/*************************************************************************/

struct KernelArgs
{
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
     * @brief Adds a range of available physical memory to the initial boot memory.
     * 
     * @remarks
     * This function expects that the KernelCode structure has been filled out and
     * will ensure that any memory added does not overlap with the kernel itself.
     * 
     * @returns True if there was space to add the memory mapping.  False on error.
     */
    bool AddFreeMemory(physical_addr_t base, size_t length);

    /// @brief Display the list of availble memory blocks.
    void ShowAvailableMemory(void);
};

/*************************************************************************/

#endif /* __FOONIX_KERNEL_ARGS_H__ */

/*************************************************************************/
