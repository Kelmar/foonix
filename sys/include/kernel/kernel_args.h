/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_ARGS_H__
#define __FOONIX_KERNEL_ARGS_H__

/*************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <utility>

/*************************************************************************/

struct MemoryRange
{
    uint32_t Base;
    size_t Length;

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

    size_t MemoryMapEntries;

    /**
     * @brief List of available memory in the first 4MB of address space.
     */
    MemoryRange MemoryMap[MAX_MEMORY_ENTRIES];
};

/*************************************************************************/

#endif /* __FOONIX_KERNEL_ARGS_H__ */

/*************************************************************************/
