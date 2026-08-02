/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGING_H__
#define __FOONIX_VM_PAGING_H__

/********************************************************************************************************************/

#include <concepts>

#include <kernel/kernel.h>
#include <kernel/utilities.h>

#include <kernel/arch.h>
/********************************************************************************************************************/

enum class PageFlags
{
    None = 0,

    Read = 0,
    User = 0,
    
    Write = 1 << 0,
    Execute = 1 << 1,

    Kernel = 1 << 7
};

as_flags(PageFlags);

/********************************************************************************************************************/

template <typename T>
concept IsPageTable = requires(T pt, paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    { pt.doMapPage(paddr, vaddr, flags) } -> std::same_as<Kernel::ErrorCode>;
    { pt.doUnmapPage(vaddr) } -> std::same_as<Kernel::ErrorCode>;
    { pt.doGetPhysicalPageFor(vaddr) } -> std::same_as<paddr_t>;
    { T::PageSize } -> std::same_as<const size_t &>;
};

/********************************************************************************************************************/

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

    /// @brief Check if the supplied aligned address is mapped or not.
    bool IsMapped(paddr_t paddr) const
    {
        return self()->doIsMapped(paddr);
    }

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

        size_t pages = (length % table_type::PageSize);
        length -= pages * table_type::PageSize;

        if (length > 0)
            ++pages;

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

    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, TMapped *mapped, PageFlags flags = PageFlags::None)
    {
        flags |= PageFlags::Write;

        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }

    template <typename TMapped>
    Kernel::ErrorCode MapStruct(paddr_t paddr, const TMapped *mapped, PageFlags flags = PageFlags::None)
    {
        // Do not add write flags
        // (REVIEW: Does it make sense here to CLEAR the write flag?)

        vaddr_t vaddr = reinterpret_cast<vaddr_t>(reinterpret_cast<uintptr_t>(mapped));
        return MapUnaligned(paddr, vaddr, sizeof(TMapped), flags);
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGING_H__ */

/********************************************************************************************************************/
