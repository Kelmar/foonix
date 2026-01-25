/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGING_H__
#define __FOONIX_VM_PAGING_H__

/********************************************************************************************************************/

#include <kernel/kernel.h>

#include <kernel/arch.h>

template <typename T>
class PageTableBase
{
protected:
    constexpr PageTableBase() { }
    constexpr PageTableBase(const PageTableBase &rhs) { }
    constexpr PageTableBase(PageTableBase &&rhs) { }

    inline constexpr T *self() { return static_cast<T *>(this); }
    inline constexpr const T *self() const { return static_cast<const T *>(this); }

public:
    virtual ~PageTableBase() { }

    Kernel::ErrorCode MapPage(paddr_t paddr, vaddr_t vaddr, uint32_t flags)
    {
        return self()->doMapPage(paddr, vaddr, flags);
    }

    Kernel::ErrorCode UnmapPage(vaddr_t vaddr)
    {
        return self()->doUnmapPage(vaddr);
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGING_H__ */

/********************************************************************************************************************/
