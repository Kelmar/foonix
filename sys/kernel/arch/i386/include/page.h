/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>

#include "cpu.h"

namespace paging
{
    static const int PAGING_TABLE_SIZE = 1024;

    /************************************************************************************************************/

    /// @brief Flags for page table entries.
    namespace page_flags
    {
        constexpr const uint32_t
            present   = 0x00000001, // Set if this page is present.
            writable  = 0x00000002, // Set if this is a writable page.
            user      = 0x00000004, // Set if this is a user space page.
            writethru = 0x00000008,
            nocache   = 0x00000010, // Set if cache is disabled
            accessed  = 0x00000020, // Set by CPU if page has been accessed.
            dirty     = 0x00000040, // Set by CPU if page has been written to.
            reserved  = 0x00000080, // Always set to zero!
            global    = 0x00000100,
            sys_bit1  = 0x00000200, // For use by OS
            sys_bit2  = 0x00000400, // For use by OS
            sys_bit3  = 0x00000800, // For use by OS

            addr_mask = 0xFFFFF000  // Physical address (shifted right 12 bits)
        ;
    }

    /// @brief Single entry in the page table
    typedef uint32_t page_entry_t;

    /// @brief Actual page table
    typedef page_entry_t page_table_t[PAGING_TABLE_SIZE];

    /************************************************************************************************************/

    /// @brief Flags for page directorys.
    namespace directory_flags
    {
        constexpr const uint32_t
            present    = 0x00000001, // Set if this page is present.
            writable   = 0x00000002, // Set if this is a writable page.
            user       = 0x00000004, // Set if this is a user space page.
            writethru  = 0x00000008,
            no_cache   = 0x00000010, // Set if cache is disabled
            accessed   = 0x00000020, // Set by CPU if page has been accessed.
            reserved   = 0x00000040, // Always set to zero!
            large_page = 0x00000080, // 0 for 4 KiB pages, 1 for 4 MiB pages
            global     = 0x00000100, // Ignored, set to zero
            sys_bit1   = 0x00000200, // For use by OS
            sys_bit2   = 0x00000400, // For use by OS
            sys_bit3   = 0x00000800, // For use by OS

            addr_mask  = 0xFFFFF000  // Page table address (shifted right 12 bits)
        ;
    }

    /// @brief Single entry in the page directory
    typedef uint32_t page_directory_entry_t;

    /// @brief Actual page directory
    typedef page_directory_entry_t page_directory_t[PAGING_TABLE_SIZE];

    /************************************************************************************************************/

    /// @brief Round address down to current page boundary.
    /// @param ptr The address to round.
    /// @return The page boundary of the supplied address.
    inline constexpr 
    uintptr_t AlignFloor(uintptr_t ptr)
    {
        return ptr & ~(PAGE_SIZE - 1);
    }

    /// @brief Round address down to current page boundary.
    /// @param ptr The address to round.
    /// @return The page boundary of the supplied address.
    inline constexpr
    uintptr_t AlignFloor(void *ptr)
    {
        return AlignFloor(reinterpret_cast<uintptr_t>(ptr));
    }

    /// @brief Round address up to next page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    template <typename T>
    inline constexpr
    T *AlignFloor(void *ptr) noexcept
    {
        return reinterpret_cast<T *>(AlignFloor(ptr));
    }

    /************************************************************************************************************/

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only round if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    inline constexpr
    uintptr_t AlignCeiling(uintptr_t ptr)
    {
        return AlignFloor(ptr + (PAGE_SIZE - 1));
    }

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only round if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    inline constexpr
    uintptr_t AlignCeiling(void *ptr)
    {
        return AlignCeiling(reinterpret_cast<uintptr_t>(ptr));
    }

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only round if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    template <typename T>
    inline constexpr
    T *AlignCeiling(void *ptr) noexcept
    {
        return reinterpret_cast<T *>(AlignCeiling(ptr));
    }

    /************************************************************************************************************/
    
    /// @brief Get the next page boundary.
    /// @remarks Unlike @ref AlignCeiling this will always return the next page.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    inline constexpr
    uintptr_t AlignNext(uintptr_t ptr)
    {
        return AlignFloor(ptr + PAGE_SIZE);
    }

    /// @brief Round address up to next page boundary.
    /// @remarks Unlike @ref AlignCeiling this will always return the next page.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    inline constexpr
    uintptr_t AlignNext(void *ptr)
    {
        return AlignNext(reinterpret_cast<uintptr_t>(ptr));
    }

    /// @brief Round address up to next page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    template <typename T>
    inline constexpr
    T *AlignNext(void *ptr) noexcept
    {
        return reinterpret_cast<T *>(AlignNext(ptr));
    }

    /************************************************************************************************************/

    /// @brief Get the previous page boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous page.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    inline constexpr
    uintptr_t AlignPrev(uintptr_t ptr)
    {
        return AlignFloor(ptr - PAGE_SIZE);
    }

    /// @brief Round address up to previous page boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous page.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    inline constexpr
    uintptr_t AlignPrev(void *ptr)
    {
        return AlignPrev(reinterpret_cast<uintptr_t>(ptr));
    }

    /// @brief Round address up to previous page boundary.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    template <typename T>
    inline constexpr
    T *AlignPrev(void *ptr) noexcept
    {
        return reinterpret_cast<T *>(AlignPrev(ptr));
    }

    /************************************************************************************************************/

    /// @brief Checks to see if the supplied pointer is page alligned.
    inline constexpr
    bool IsAligned(uintptr_t ptr)
    {
        uintptr_t x = AlignFloor(ptr);
        return x == ptr;
    }

    /// @brief Checks to see if the supplied pointer is page alligned.
    inline constexpr
    bool IsAligned(void *ptr)
    {
        uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t x = AlignFloor(p);
        return p == x;
    }

    /************************************************************************************************************/

    /// @brief Map a phyiscal memory page to a virtual memory page.
    /// @remark Note that addresses and sizes might get aligned to processor page boundaries.
    /// @param dir Directory to map the page in.
    /// @param paddr The phyiscal address to be mapped
    /// @param vaddr The virtual address
    /// @param flags Flags to be set on the page (The present flag is added automatically.)
    Kernel::ErrorCode MapPage(page_directory_t dir, paddr_t paddr, vaddr_t vaddr, uint32_t flags);

    /// @brief Remove a virtual page from paging.
    /// @param dir Directory to unmap from.
    /// @param vaddr The virtual address to unmap.
    void UnmapPage(page_directory_t dir, vaddr_t vaddr);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_PAGE_H__ */

/********************************************************************************************************************/
