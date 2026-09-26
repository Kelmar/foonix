/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__
#define __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__

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

    constexpr const size_t EntryCount = 512;
    
    typedef uint64_t pml4t_t;
    typedef uint64_t pdpt_t;
    typedef uint64_t pde_t;
    typedef uint64_t pte_t;

    /************************************************************************************************************/

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        static constexpr size_t MaxLevels = 4;

        uint64_t *m_topLevel;

        paddr_t AllocatePage();

        static
        uint64_t MapToEntryFlags(PageFlags flags);

        void MapSinglePage(paddr_t paddr, vaddr_t vaddr, uint64_t entryFlags);

    public:
        PageTable() noexcept;

        virtual ~PageTable() { }

        kernel::ErrorCode doMapPages(paddr_t paddr, vaddr_t vaddr, size_t count, PageFlags flags);

        kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;

        void doMakeActive() const { x86::load_cr3(reinterpret_cast<uintptr_t>(m_topLevel)); }
    };

    /************************************************************************************************************/

    void Init(memory::VPageMapBuilder &);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__ */

/********************************************************************************************************************/
