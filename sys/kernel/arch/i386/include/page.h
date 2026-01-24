/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>
#include <kernel/utilities.h>

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
    // Page alignment utilities
    
    /// @brief Round address down to current page boundary.
    /// @param ptr The address to round.
    /// @return The page boundary of the supplied address.
    const util::TAlignFloor<PAGE_SIZE> AlignFloor;

    /// @brief Round address up to page boundary.
    /// @remarks Unlike @ref AlignNext this will only rounded if we're not already on a page boundary.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignCeiling<PAGE_SIZE> AlignCeiling;
    
    /// @brief Get the next page boundary.
    /// @remarks Unlike @ref AlignCeiling this will always return the next page.
    /// @param ptr The address to round.
    /// @return The address of the next page boundary.
    const util::TAlignNext<PAGE_SIZE> AlignNext;

    /// @brief Get the previous page boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous page.
    /// @param ptr The address to round.
    /// @return The address of the previous page boundary.
    const util::TAlignPrev<PAGE_SIZE> AlignPrev;

    /// @brief Checks to see if the supplied pointer is page alligned.
    const util::TIsAligned<PAGE_SIZE> IsAligned;

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
