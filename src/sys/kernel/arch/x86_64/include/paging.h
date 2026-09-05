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
    
    typedef uint64_t pdpt_t;
    typedef uint64_t pde_t;
    typedef uint64_t pte_t;

    /************************************************************************************************************/

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        pdpt_t *m_pdpt;

    public:
        PageTable() noexcept;

        virtual ~PageTable() { }

        kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;

        void doMakeActive() const { x86::load_cr3(reinterpret_cast<uintptr_t>(m_pdpt)); }
    };

    /************************************************************************************************************/

    void Init(memory::VPageMapBuilder &);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__ */

/********************************************************************************************************************/
