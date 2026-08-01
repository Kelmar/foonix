/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGING_H__
#define __FOONIX_VM_PAGING_H__

/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/utilities.h>

#include <kernel/arch.h>

// TODO: Remove this later.
#ifndef PAGE_SIZE
# define PAGE_SIZE 4096
#endif

template <typename T>
class PageTableBase
{
protected:
    constexpr PageTableBase() { }

    constexpr PageTableBase(const PageTableBase &rhs) = delete;
    constexpr PageTableBase(PageTableBase &&rhs) = delete;

    //constexpr PageTableBase(const PageTableBase &rhs) { }
    //constexpr PageTableBase(PageTableBase &&rhs) { }

    inline constexpr T *self() { return static_cast<T *>(this); }
    inline constexpr const T *self() const { return static_cast<const T *>(this); }

public:
    virtual ~PageTableBase() { }

    /// @brief Check if the supplied aligned address is mapped or not.
    bool IsMapped(paddr_t paddr) const
    {
        return self()->doIsMapped(paddr);
    }

    /// @brief Map a aligned physical page to an aligned virtual page.
    Kernel::ErrorCode MapPage(paddr_t paddr, vaddr_t vaddr, uint32_t flags)
    {
        return self()->doMapPage(paddr, vaddr, flags);
    }

    /// @brief Unmap an aligned page.
    Kernel::ErrorCode UnmapPage(vaddr_t vaddr)
    {
        return self()->doUnmapPage(vaddr);
    }

    /**
     * @brief Map possibly unaligned contiguous physical pages to virtual pages.
     *
     * @param paddr - First physical address to align
     * @param vaddr - First virtual address to align
     * @param length - Number of bytes to map.
     *
     * @remarks The function will always map in pages.
     */
    Kernel::ErrorCode MapUnaligned(paddr_t paddr, vaddr_t vaddr, size_t length, uint32_t flags)
    {
        paddr_t p_aligned = util::AlignFloor<PAGE_SIZE>(paddr);
        paddr_t v_aligned = util::AlignFloor<PAGE_SIZE>(vaddr);

        size_t pages = (length % PAGE_SIZE);
        length -= pages * PAGE_SIZE;

        if (length > 0)
            ++pages;

        for (size_t i = 0; i < pages; ++i)
        {
            auto result = MapPage(p_aligned, v_aligned, flags);

            if (result != Kernel::ErrorCode::NoError)
                return result;

            p_aligned += PAGE_SIZE;
            v_aligned += PAGE_SIZE;
        }
        
        return Kernel::ErrorCode::NoError;
    }

    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, TMapped *mapped, uint32_t flags)
    {
        // TODO: Add writable flags
        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }

    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, const TMapped *mapped, uint32_t flags)
    {
        // Do not add writable flag.

        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGING_H__ */

/********************************************************************************************************************/
