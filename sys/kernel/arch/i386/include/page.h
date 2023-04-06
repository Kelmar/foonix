/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_PAGE_H__
#define __FOONIX_KERNEL_ARCH_X86_PAGE_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>

#include "cpu.h"

namespace Arch::Paging
{
    static const int PAGING_TABLE_SIZE = 1024;

    /// @brief Single entry in the page table
    struct page_entry_t
    {
        uint32_t present  : 1;  // Set if this page is present.
        uint32_t writable : 1;  // Set if this is a writable page.
        uint32_t user     : 1;  // Set if this is a user space page.
        uint32_t writethu : 1;
        uint32_t nocache  : 1;  // Set if cache is disabled
        uint32_t accessed : 1;  // Set by CPU if page has been accessed.
        uint32_t dirty    : 1;  // Set by CPU if page has been written to.
        uint32_t reserved : 1;  // Always set to zero!
        uint32_t global   : 1;
        uint32_t sys_bits : 3;  // For use by OS
        uint32_t addr     : 20; // Physical address (shifted right 12 bits)
    } __attribute__((packed));

    /// @brief Actual page table
    struct page_table_t
    {
        page_entry_t table[PAGING_TABLE_SIZE];
    };

    /**
     * @brief Single entry in the page directory
     * @remark This is for a 4KByte directory entry.
     */ 
    struct page_directory_entry_t
    {
        uint32_t present    : 1;  // Set if this page is present.
        uint32_t writable   : 1;  // Set if this is a writable page.
        uint32_t user       : 1;  // Set if this is a user space page.
        uint32_t writethu   : 1;
        uint32_t nocache    : 1;  // Set if cache is disabled
        uint32_t accessed   : 1;  // Set by CPU if page has been accessed.
        uint32_t reserved   : 1;  // Always set to zero!
        uint32_t pagesize   : 1;  // 0 for 4KByte
        uint32_t global     : 1;  // Ignored, set to zero
        uint32_t sys_bits   : 3;  // For use by OS
        uint32_t page_table : 20; // Page table address (shifted right 12 bits)
    } __attribute__((packed));

    /// @brief Actual page directory
    struct page_directory_t
    {
        page_directory_entry_t tables[PAGING_TABLE_SIZE];
    } __attribute__((packed));

    /************************************************************************************************************/
    /// @brief Map a physical address to the kernel's boot paging logical address
    template <typename T>
    T *MapToBootLogical(physical_addr_t physical)
    {
        if (physical >= 1024 * 1024)
            physical += RELOCATE_START;

        return reinterpret_cast<T *>(physical);
    }

    /************************************************************************************************************/
    /// @brief Map a boot logical address to a physical address
    template <typename T>
    physical_addr_t MapFromBootLogical(T *ptr)
    {
        logical_addr_t addr = reinterpret_cast<logical_addr_t>(ptr);

        if (addr >= 1024 * 1024)
            addr -= RELOCATE_START;

        return reinterpret_cast<physical_addr_t>(addr);
    }

    /************************************************************************************************************/
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_PAGE_H__ */

/********************************************************************************************************************/
