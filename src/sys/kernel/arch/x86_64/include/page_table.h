/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__
#define __FOONIX_KERNEL_ARCH_X64_PAGE_TABLE_H__

/********************************************************************************************************************/

namespace paging
{
    constexpr const size_t EntryCount = 512;
    
    typedef uint64_t pdpt_t;
    typedef uint64_t pde_t;
    typedef uint64_t pte_t;
    
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
