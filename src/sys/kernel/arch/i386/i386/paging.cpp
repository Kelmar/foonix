/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel_args.h>
#include <kernel/debug.h>

#include <kernel/vm.h>

#include <kernel/arch.h>

#include "asm.h"
#include "atomic.h"
#include "cpu.h"
#include "cpudefs.h"
#include "multiboot.h"
#include "arch_vm.h"
#include "page.h"

/********************************************************************************************************************/

using namespace paging;

namespace
{
    /********************************************************************************************************************/

#if 0
    constexpr uint32_t MapToDirFlags(PageFlags flags)
    {
        uint32_t rval = directory_flags::user;

        if (has_flags(flags, PageFlags::Write))
            rval |= directory_flags::writable;

        if (has_flags(flags, PageFlags::Kernel))
            rval &= ~directory_flags::user;

        return rval;
    }
#endif

    /********************************************************************************************************************/

    constexpr uint32_t MapToPageFlags(PageFlags flags)
    {
        uint32_t rval = page_flags::user;

        if (has_flags(flags, PageFlags::Write))
            rval |= page_flags::writable;

        if (has_flags(flags, PageFlags::Kernel))
            rval &= ~page_flags::user;

        return rval;
    }

#if 0
    /************************************************************************************************************/
    /**
     * @brief Return the number of available entires in the page table.
     */
    uint32_t GetFreeTableEntries(page_table_t *pageTable)
    {
        uint32_t rval = 0;

        for (size_t i = 0; i < PAGING_TABLE_SIZE; ++i)
        {
            page_entry_t *entry = &pageTable->table[i];

            if (!entry->present)
                ++rval;
        }

        return rval;
    }

    /************************************************************************************************************/
    /**
     * @brief Returns the index of a free page table entry.
     * @returns -1 if an entry cannot be found.  Otherwise a page table index.
     * @remark Does not allocate memory
     */
    int GetFreePageTableEntry(page_table_t *pageTable, int start, Kernel::Direction direction = Kernel::Direction::Up)
    {
        int dir = direction == Kernel::Direction::Up ? 1 : -1;

        if (start >= PAGING_TABLE_SIZE || start < 0)
            start = direction == Kernel::Direction::Up ? 0 : (PAGING_TABLE_SIZE - 1);

        for (; start >= 0 && start < PAGING_TABLE_SIZE; start += dir)
        {
            if (!pageTable->table[start].present)
                return start;
        }

        return -1;
    }

    /************************************************************************************************************/
    /**
     * @brief Get a free table entry index in a directory.
     * @returns -1 if a entry couldn't be found
     * @remark Does not allocate memory
     */
    page_index_t GetFreeTableEntry(page_directory_t *pageDir, page_index_t start, Kernel::Direction direction = Kernel::Direction::Up)
    {
        int dir = direction == Kernel::Direction::Up ? 1 : -1;
        int dirIndex = start >> 10;

        if (dirIndex >= PAGING_TABLE_SIZE || dirIndex < 0)
            dirIndex = direction == Kernel::Direction::Up ? 0 : (PAGING_TABLE_SIZE - 1);

        for (;dirIndex < PAGING_TABLE_SIZE; dirIndex += dir)
        {
            if (!pageDir->tables[dirIndex].present)
                continue;

            int val = pageDir->tables[dirIndex].page_table;
            page_table_t *table = reinterpret_cast<page_table_t*>(val << 12);

            int pageIndex = GetFreePageTableEntry(table, start & (PAGING_TABLE_SIZE - 1), direction);

            if (pageIndex == -1)
                continue;

            return static_cast<page_index_t>((dirIndex << 10) | pageIndex);
        }

        return -1;
    }

    /************************************************************************************************************/
    /**
     * @brief Locate where we can insert a new page table.
     */
    int GetFreeDirectoryIndex(page_directory_t *pageDir, int start, Kernel::Direction direction = Kernel::Direction::Up)
    {
        int dir = direction == Kernel::Direction::Up ? 1 : -1;
        int rval = (start < PAGING_TABLE_SIZE) && (start > 0) ? start : 0;

        for (; rval >= 0 && rval < PAGING_TABLE_SIZE; rval += dir)
        {
            if (!pageDir->tables[rval].present)
                return rval;
        }

        return -1;
    }
#endif

    /************************************************************************************************************/
    /**
     * @brief Convert a page_directory_entry into a page_table_t.
     *
     * @remarks Effectively just strips the flags off the page_directory_entry_t value.
     */
    inline
    page_entry_t *GetPageTable(const page_directory_entry_t &pde)
    {
        /*
         * TODO: The pointer in the directory entry is the physical address, find a good
         * way we can map that into the kernel space so we can update it.
         *
         * Right now we have it hard coded to our PHYS_2_VIRT macro.
         */

        uintptr_t ptr = pde & directory_flags::addr_mask;
        return reinterpret_cast<page_entry_t *>(PHYS_2_VIRT(ptr));
    }

    /************************************************************************************************************/

    /**
     * @brief Check if a physical address is mapped in a page_directory_t.
     */
    bool IsMapped(const paging::page_directory_t dir, paddr_t paddr)
    {
        (void)(dir);
        (void)(paddr);

        return false;
    }

    /************************************************************************************************************/
    /**
     * @brief Map a phyiscal memory page to a virtual memory page.
     * @remark Note that addresses and sizes might get aligned to processor page boundaries.
     * @param dir Directory to map the page in.
     * @param paddr The phyiscal address to be mapped
     * @param vaddr The virtual address
     * @param flags Flags to be set on the page (The present flag is added automatically.)
     */
    Kernel::ErrorCode MapPage(paging::page_directory_t dir, paddr_t paddr, vaddr_t vaddr, PageFlags flags)
    {
        if (!IsAligned(paddr) || !IsAligned(vaddr))
            return Kernel::ErrorCode::NotAligned;

        //uint32_t dirFlags = MapToDirFlags(flags);
        uint32_t pageFlags = MapToPageFlags(flags);

        //Debug::PrintF("Map %p -> %p\r\n", physEntry, virtEntry);
        
        int pgtIndex = (vaddr >> 12) & 0x03FF;
        int dirIndex = (vaddr >> 22) & 0x03FF;

        page_directory_entry_t &pde = dir[dirIndex];

        // Assert these entries are correct!
        if ((pde & directory_flags::present) == 0)
            kpanic("Request to map to non present page entry!");

        page_entry_t *pageTable = GetPageTable(pde);
        page_entry_t &page = pageTable[pgtIndex];

        if ((page & page_flags::present) != 0)
        {
            auto maskedPtr = page & page_flags::addr_mask;

            Debug::PrintF("WARNING: Page over writing: %p with %p\r\n", maskedPtr, paddr);
        }

        page = (paddr & page_flags::addr_mask) | pageFlags | page_flags::present;

        return Kernel::ErrorCode::NoError;
    }

    /************************************************************************************************************/
    /**
     * @brief Remove a virtual page from paging.
     * @param dir Directory to unmap from.
     * @param vaddr The virtual address to unmap.
     */
    Kernel::ErrorCode UnmapPage(paging::page_directory_t dir, vaddr_t vaddr)
    {
        if (!IsAligned(vaddr))
            return Kernel::ErrorCode::NotAligned;
        
        int pgtIndex = (vaddr >> 12) & 0x03FF;
        int dirIndex = (vaddr >> 22) & 0x03FF;

        page_directory_entry_t &pde = dir[dirIndex];

        // Assert these entries are correct!
        if ((pde & directory_flags::present) == 0)
            return Kernel::ErrorCode::NoError; // Nothing to do

        page_entry_t *pageTable = GetPageTable(pde);
        page_entry_t &page = pageTable[pgtIndex];

        if ((page & page_flags::present) == 0)
            return Kernel::ErrorCode::NoError; // Nothing to do

        page &= ~page_flags::present;

        return Kernel::ErrorCode::NoError;
    }

    /************************************************************************************************************/

    paddr_t GetPhysicalPageFor(paging::page_directory_t dir, vaddr_t vaddr)
    {
        int pgtIndex = (vaddr >> 12) & 0x03FF;
        int dirIndex = (vaddr >> 22) & 0x03FF;

        page_directory_entry_t &pde = dir[dirIndex];

        // Assert these entries are correct!
        if ((pde & directory_flags::present) == 0)
            return 0; // Not mapped

        page_entry_t *pageTable = GetPageTable(pde);
        page_entry_t &page = pageTable[pgtIndex];
        
        if ((page & page_flags::present) == 0)
            return 0; // Not mapped

        return reinterpret_cast<paddr_t>(page & page_flags::addr_mask);
    }
}

/********************************************************************************************************************/

// Defined in start.S
extern "C" page_directory_t boot_page_directory;
extern "C" page_table_t boot_page_identity;
extern "C" page_table_t boot_page_kernel;

BootPageTable paging::g_bootPageTable;

/********************************************************************************************************************/

#if 0
Kernel::ErrorCode paging::InitPaging(KernelArgs *ka)
{
    UNUSED(ka);

    int tableIndex = -1;
    int dirIndex = 0;

    for (; dirIndex < PAGING_TABLE_SIZE; ++dirIndex)
    {
        if (bootDir->tables[dirIndex].present)
        {
            page_index_t page = 0;
            tableIndex = GetFreePageTableEntry()
        }
    }

    if (!freeIndex)
    {
        // Edge case
        Debug::PrintF("Could not find free page table!\r\n");
        return Kernel::ErrorCode::Unknown;
    }
    
    // Allocate a new page
    page_index_t newPage = VM::AllocRawPage();

    if (newPage == 0)
        return Kernel::ErrorCode::OutOfMemory;

    // Map the new page table into our boot directory:
    
    return Kernel::ErrorCode::NoError;
}

#endif

/********************************************************************************************************************/
/********************************************************************************************************************/

PageTable::PageTable()
    : PageTableBase()
{
    memset(m_dir, 0, sizeof(page_directory_t));
}

/********************************************************************************************************************/

bool PageTable::doIsMapped(paddr_t paddr) const
{
    return ::IsMapped(m_dir, paddr);
}

/********************************************************************************************************************/

Kernel::ErrorCode PageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    return ::MapPage(m_dir, paddr, vaddr, flags);
}

/********************************************************************************************************************/

Kernel::ErrorCode PageTable::doUnmapPage(vaddr_t vaddr)
{
    return ::UnmapPage(m_dir, vaddr);
}

/********************************************************************************************************************/

paddr_t PageTable::doGetPhysicalPageFor(vaddr_t addr)
{
    return ::GetPhysicalPageFor(m_dir, addr);
}

/********************************************************************************************************************/
/********************************************************************************************************************/

bool BootPageTable::doIsMapped(paddr_t paddr) const
{
    return ::IsMapped(boot_page_directory, paddr);
}

/********************************************************************************************************************/

Kernel::ErrorCode BootPageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    return ::MapPage(boot_page_directory, paddr, vaddr, flags);
}

/********************************************************************************************************************/

Kernel::ErrorCode BootPageTable::doUnmapPage(vaddr_t vaddr)
{
    return ::UnmapPage(boot_page_directory, vaddr);
}

/********************************************************************************************************************/

paddr_t BootPageTable::doGetPhysicalPageFor(vaddr_t addr)
{
    return ::GetPhysicalPageFor(boot_page_directory, addr);
}

/********************************************************************************************************************/
