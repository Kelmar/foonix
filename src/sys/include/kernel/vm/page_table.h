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
    { T::PageSize } -> std::same_as<const size_t &>;
    { pt.doMapPage(paddr, vaddr, flags) } -> std::same_as<Kernel::ErrorCode>;
    { pt.doUnmapPage(vaddr) } -> std::same_as<Kernel::ErrorCode>;
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
    constexpr PageTableBase() { }

    constexpr PageTableBase(const PageTableBase &rhs) = delete;
    constexpr PageTableBase(PageTableBase &&rhs) = delete;

    //constexpr PageTableBase(const PageTableBase &rhs) { }
    //constexpr PageTableBase(PageTableBase &&rhs) { }

    inline constexpr T *self() { return static_cast<T *>(this); }
    inline constexpr const T *self() const { return static_cast<const T *>(this); }

public:
    virtual ~PageTableBase() { }

    /// @brief Map a aligned physical page to an aligned virtual page.
    Kernel::ErrorCode MapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags = PageFlags::None)
    {
        static_assert(IsPageTable<table_type>);
        return self()->doMapPage(paddr, vaddr, flags);
    }

    /// @brief Unmap an aligned page.
    Kernel::ErrorCode UnmapPage(vaddr_t vaddr)
    {
        static_assert(IsPageTable<table_type>);
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
        static_assert(IsPageTable<table_type>);
        vaddr_t vpage = util::AlignFloor<table_type::PageSize>(vaddr);
        return self()->doGetPhysicalPageFor(vpage);
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
    Kernel::ErrorCode MapUnaligned(paddr_t paddr, vaddr_t vaddr, size_t length, PageFlags flags = PageFlags::None)
    {
        static_assert(IsPageTable<table_type>);

        paddr_t p_aligned = util::AlignFloor<table_type::PageSize>(paddr);
        paddr_t v_aligned = util::AlignFloor<table_type::PageSize>(vaddr);

        size_t pages = (length / table_type::PageSize);
        size_t extra = (length % table_type::PageSize);

        if (extra > 0)
            ++pages;

        //Debug::PrintF("Request to map %d bytes == %d page(s)\r\n", length, pages);

        for (size_t i = 0; i < pages; ++i)
        {
            auto result = MapPage(p_aligned, v_aligned, flags);

            if (result != Kernel::ErrorCode::NoError)
                return result;

            p_aligned += table_type::PageSize;
            v_aligned += table_type::PageSize;
        }
        
        return Kernel::ErrorCode::NoError;
    }

    /**
     * @brief Maps a structure to a physical page.
     *
     * @param paddr  - The physical address of the structure.
     * @param mapped - The desired virtual address of the structure.
     * @param flags  - Access and other modifier flags for the structure.
     *
     * @remarks This is a convenience function to MapUnaligned() that also does any needed casting
     * to get pointer into a valid vaddr_t type to the MapUnaligned() call.
     *
     * Note that this function will automatically add the PageFlags::Write flag.
     *
     * @return Returns a status code indicating the success or failure of the mapping.
     */
    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, TMapped *mapped, PageFlags flags = PageFlags::None)
    {
        flags |= PageFlags::Write;

        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }

    /**
     * @brief Maps a structure to a physical page.
     *
     * @param paddr  - The physical address of the structure.
     * @param mapped - The desired virtual address of the structure.
     * @param flags  - Access and other modifier flags for the structure.
     *
     * @remarks This is a convenience function to MapUnaligned() that also does any needed casting
     * to get pointer into a valid vaddr_t type to the MapUnaligned() call.
     *
     * This version of the function will NOT add the PageFlags::Write flag.
     *
     * @return Returns a status code indicating the success or failure of the mapping.
     */
    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, const TMapped *mapped, PageFlags flags = PageFlags::None)
    {
        // Do not add write flags
        // (REVIEW: Does it make sense here to CLEAR the write flag?)

        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }

    /**
     * @brief Sets the page table as the currently active page table for the MMU.
     */
    void MakeActive() const { self()->doMakeActive(); }
};

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGE_TABLE_H__ */

/********************************************************************************************************************/
