/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGE_TABLE_H__
#define __FOONIX_VM_PAGE_TABLE_H__

/********************************************************************************************************************/

#include <concepts>

#include <kernel/kernel.h>
#include <kernel/utilities.h>

#include "cpu.h"

/********************************************************************************************************************/

namespace memory
{
    class VPageMapBuilder;
}

namespace paging
{
    /************************************************************************************************************/
    // Page alignment utilities
    
    /// @brief Round address down to current page boundary.
    /// @param ptr The address to round.
    /// @return The page boundary of the supplied address.
    const util::TAlignFloor<cpu::PageSize> AlignFloor;

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only rounded if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignCeiling<cpu::PageSize> AlignCeiling;
    
    /// @brief Get the next page boundary.
    /// @remarks Unlike @ref AlignCeiling this will always return the next page.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignNext<cpu::PageSize> AlignNext;

    /// @brief Get the previous page boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous page.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    const util::TAlignPrev<cpu::PageSize> AlignPrev;

    /// @brief Checks to see if the supplied pointer is page aligned.
    const util::TIsAligned<cpu::PageSize> IsAligned;

    /// @brief Compute the minium number of pages needed to store the supplied number of bytes.
    const util::TMinPages<cpu::PageSize> MinPages;

    /// @brief Architecture specific kernel VPageMap overrides.
     //kernel::ErrorCode init_vmap(VPageMapBuilder &);
}

/********************************************************************************************************************/
/**
 * @brief Flags for pages.
 */
enum class PageFlags
{
    /// @brief No special flags mapped for this page.
    None = 0,

    /// @brief Page should be readable.
    Read = 0,

    /// @brief User access allowed for page.
    User = 0,
    
    /// @brief Writing to the page should be enabled.
    Write   = 1 << 0,

    /// @brief The page should be allowed to execute code from the page.
    Execute = 1 << 1,

    /// @brief The page is for Kernel access only, prevent access to User level code.
    Kernel  = 1 << 7,
};

as_flags(PageFlags);

/********************************************************************************************************************/
/**
 * @brief Concept to enforce interface to architecture items.
 */
template <typename T>
concept IsPageTable = requires(T pt, paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    { pt.doMapPage(paddr, vaddr, flags) } -> std::same_as<kernel::ErrorCode>;
    { pt.doUnmapPage(vaddr) } -> std::same_as<kernel::ErrorCode>;
    { pt.doGetPhysicalPageFor(vaddr) } -> std::same_as<paddr_t>;
    { pt.doMakeActive() } -> std::same_as<void>;
};

/********************************************************************************************************************/
/**
 * @brief Base implementation for page tables.
 */
template <typename T>
class PageTableBase
{
public:
    typedef T table_type;

protected:
    constexpr PageTableBase() noexcept
    {
        static_assert(IsPageTable<table_type>);
    }

    constexpr PageTableBase(const PageTableBase &rhs) = delete;
    constexpr PageTableBase(PageTableBase &&rhs) = delete;

    inline constexpr T *self() { return static_cast<T *>(this); }
    inline constexpr const T *self() const { return static_cast<const T *>(this); }

public:
    virtual ~PageTableBase() { }

    /// @brief Map a aligned physical page to an aligned virtual page.
    kernel::ErrorCode MapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags = PageFlags::None)
    {
        return self()->doMapPage(paddr, vaddr, flags);
    }

    /// @brief Unmap an aligned page.
    kernel::ErrorCode UnmapPage(vaddr_t vaddr)
    {
        return self()->doUnmapPage(vaddr);
    }

    /**
     * @brief Walk page table and return the physical page mapped for the given virtual address.
     *
     * @param vaddr The virtual address to find the page of.  It need not be aligned.
     *
     * @return The physical address found for the virtual page, or nullptr if not found.
     */
    inline
    paddr_t GetPhysicalPageFor(vaddr_t vaddr)
    {
        vaddr_t vpage = paging::AlignFloor(vaddr);
        return self()->doGetPhysicalPageFor(vpage);
    }

    /**
     * @brief Sets the page table as the currently active page table for the MMU.
     */
    void MakeActive() const { self()->doMakeActive(); }
};

/********************************************************************************************************************/

namespace paging
{
    class PageTable; // Defined by architecture.
};

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGE_TABLE_H__ */

/********************************************************************************************************************/
