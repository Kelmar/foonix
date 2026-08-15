/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <type_traits>

#include <kernel/kernel.h>
#include <kernel/utilities.h>

#include <kernel/vm.h>

#include "cpu.h"

namespace paging
{
    static const int PAGING_TABLE_SIZE = 1024;

    /************************************************************************************************************/

    /// @brief Single entry in the page table
    typedef uint32_t page_entry_t;

    /// @brief Actual page table
    typedef page_entry_t page_table_t[PAGING_TABLE_SIZE];

    /************************************************************************************************************/

    /// @brief Single entry in the page directory
    typedef uint32_t page_directory_entry_t;

    /// @brief Actual page directory
    typedef page_directory_entry_t page_directory_t[PAGING_TABLE_SIZE];

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
        page_directory_t m_dir;

        PageTable(const PageTable &rhs) = delete;
        PageTable(PageTable &&rhs) = delete;

    public:
        static constexpr const size_t PageSIze = 4096;

        PageTable();
        
        virtual ~PageTable() { }

        bool doIsMapped(paddr_t addr) const;

        Kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        Kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;
    };

    /************************************************************************************************************/

    class BootPageTable : public PageTableBase<BootPageTable>
    {
    private:
        BootPageTable(const BootPageTable &rhs) = delete;
        BootPageTable(BootPageTable &&rhs) = delete;

    public:
        static constexpr const size_t PageSize = 4096;

        constexpr BootPageTable() : PageTableBase() { }

        virtual ~BootPageTable() { }

        bool doIsMapped(paddr_t addr) const;

        Kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        Kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr);
    };

    extern BootPageTable g_bootPageTable;

    /************************************************************************************************************/
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_PAGE_H__ */

/********************************************************************************************************************/
