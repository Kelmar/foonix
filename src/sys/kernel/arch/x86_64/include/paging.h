/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__
#define __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__

/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/utilities.h>

#include <kernel/vm.h>

#include "cpu.h"

/********************************************************************************************************************/

namespace paging
{
    /************************************************************************************************************/

    constexpr const size_t EntryCount = 512;
    
    typedef uint64_t pdpt_t;
    typedef uint64_t pde_t;
    typedef uint64_t pte_t;
    
    /************************************************************************************************************/
    // Page alignment utilities
    
    /// @brief Round address down to current page boundary.
    /// @param ptr The address to round.
    /// @return The page boundary of the supplied address.
    const util::TAlignFloor<PAGE_SIZE> AlignFloor;

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only rounded if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignCeiling<PAGE_SIZE> AlignCeiling;
    
    /// @brief Get the next page boundary.
    /// @remarks Unlike @ref AlignCeiling this will always return the next page.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignNext<PAGE_SIZE> AlignNext;

    /// @brief Get the previous page boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous page.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    const util::TAlignPrev<PAGE_SIZE> AlignPrev;

    /// @brief Checks to see if the supplied pointer is page alligned.
    const util::TIsAligned<PAGE_SIZE> IsAligned;
    
    /************************************************************************************************************/

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        pdpt_t m_pdpt;

    public:
        static constexpr const size_t PageSize = 4096;

        PageTable();

        virtual ~PageTable() { }

        bool doIsMapped(paddr_t addr) const;

        Kernel::ErrorCode doMapPage(paddr_t padd, vaddr_t vaddr, PageFlags flags);
        Kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__ */

/********************************************************************************************************************/
