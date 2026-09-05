/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

//#include <kernel/kernel.h>
//#include <kernel/utilities.h>

//#include <kernel/vm.h> // We can't include this here, vm.h needs to reference us.

#include "cpu.h"

/********************************************************************************************************************/

namespace memory
{
    class VPageMapBuilder;
}

/********************************************************************************************************************/

namespace paging
{
    /************************************************************************************************************/

    /// @brief Single entry in the page table
    typedef uint32_t page_entry_t;

    /// @brief Single entry in the page directory
    typedef uint32_t page_directory_entry_t;

    /************************************************************************************************************/

    enum class DirectoryOptions
    {
        None    = 0,
        NoClear = 1,
    };

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        page_directory_entry_t *m_dir;

        PageTable(const PageTable &rhs) = delete;
        PageTable(PageTable &&rhs) = delete;

        static paddr_t AllocatePage();

        constexpr size_t ToEntryIndex(vaddr_t vaddr) const { return (vaddr >> 12) & 0x03FFF; }
        constexpr size_t ToDirIndex  (vaddr_t vaddr) const { return (vaddr >> 22) & 0x03FFF; }

        kernel::ErrorCode AddDirectoryEntry(size_t index, paddr_t table, PageFlags flags);

        page_entry_t *GetPageTable(vaddr_t vaddr, size_t &dirIndex) const;

        page_entry_t *GetPageTable(vaddr_t vaddr) const { size_t discard; return GetPageTable(vaddr, discard); }

        page_entry_t *GetOrCreatePageTable(vaddr_t vaddr, PageFlags flags);

    public:
        PageTable() noexcept;
        PageTable(page_directory_entry_t *directory, DirectoryOptions options = DirectoryOptions::None) noexcept;
        
        virtual ~PageTable() { }

        kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;

        void doMakeActive() const { x86::load_cr3(reinterpret_cast<uintptr_t>(m_dir)); }
    };

    extern PageTable g_bootPageTable;

    /************************************************************************************************************/

    void Init(memory::VPageMapBuilder &);

    /************************************************************************************************************/
}

/********************************************************************************************************************/

as_flags(paging::DirectoryOptions);

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_PAGE_H__ */

/********************************************************************************************************************/
