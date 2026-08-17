/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <type_traits>

//#include <kernel/kernel.h>
//#include <kernel/utilities.h>

//#include <kernel/vm.h> // We can't include this here, vm.h needs to reference us.

#include <kernel/kernel_args.h>

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

    enum class DirectoryOptions
    {
        None    = 0,
        NoClear = 1,
    };

    // Rework PageTable and BootPageTable into a single class, can call constructor manually with inplace new.

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        page_directory_entry_t *m_dir;

        PageTable(const PageTable &rhs) = delete;
        PageTable(PageTable &&rhs) = delete;

    public:
        static constexpr const size_t PageSize = 4096;

        constexpr PageTable() : m_dir(nullptr) { }

        PageTable(page_directory_entry_t *directory, DirectoryOptions options = DirectoryOptions::None);
        
        virtual ~PageTable() { }

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

    extern PageTable g_bootPageTable;

    /************************************************************************************************************/

    void Init(KernelArgs *ka);

    /************************************************************************************************************/
}

/********************************************************************************************************************/

as_flags(paging::DirectoryOptions);

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_PAGE_H__ */

/********************************************************************************************************************/
