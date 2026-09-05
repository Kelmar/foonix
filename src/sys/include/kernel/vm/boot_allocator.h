/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGE_ALLOCATOR_H__
#define __FOONIX_VM_PAGE_ALLOCATOR_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/types.h>

/********************************************************************************************************************/

namespace memory
{
    /// @brief A simple boot up bump allocator.
    class BootPageAllocator
    {
    private:
        paddr_t m_addr;
        size_t m_count;

    public:
        contexpr BootPageAllocator(paddr_t addr) noexcept
            : m_addr(addr)
            , m_count(0)
        {}

        /// @brief Allocate a new page from this allocator.
        inline
        paddr_t AllocatePage()
        {
            paddr_t result = m_addr;
            m_addr += cpu::PageCount;
            ++m_count;
            return result;
        }

        /// @brief Get what the next address will be from this allocator.
        inline
        paddr_t NextAddress() const { return m_addr; }

        /// @brief Get the number of pages that were allocated with this allocator.
        inline
        size_t AllocatedPages() const { return m_count; }
    };

}

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGE_ALLOCATOR_H__ */

/********************************************************************************************************************/

