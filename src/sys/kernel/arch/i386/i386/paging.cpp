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
#include "paging.h"

/********************************************************************************************************************/

using namespace paging;

namespace
{
    /************************************************************************************************************/

    /// @brief Flags for page directory entries.
    namespace DirEntryFlags
    {
        constexpr const uint32_t
            Present        = 0x00000001, // Set if this page is present.
            Writable       = 0x00000002, // Set if this is a writable page.
            User           = 0x00000004, // Set if this is a user space page.
            WriteThruCache = 0x00000008,
            DisableCache   = 0x00000010, // Set if cache is disabled
            Accessed       = 0x00000020, // Set by CPU if page has been accessed.
            Reserved       = 0x00000040, // Always set to zero!
            LargeEntry     = 0x00000080, // 0 for 4 KiB pages, 1 for 4 MB pages
            Global         = 0x00000100, // Ignored, set to zero (used for 4MB pages)

            SystemBit1     = 0x00000200, // For use by OS
            SystemBit2     = 0x00000400, // For use by OS
            SystemBit3     = 0x00000800, // For use by OS

            AddressMask    = 0xFFFFF000  // Page table address (shifted right 12 bits)
        ;
    }

    /************************************************************************************************************/

    /// @brief Flags for page directorys entries in 4MB mode.
    namespace DirEntryFlags_4MB
    {
        constexpr const uint32_t
            Present        = 0x00000001, // Set if this page is present.
            Writable       = 0x00000002, // Set if this is a writable page.
            User           = 0x00000004, // Set if this is a user space page.
            WriteThruCache = 0x00000008,
            DisableCache   = 0x00000010, // Set if cache is disabled
            Accessed       = 0x00000020, // Set by CPU if page has been accessed.
            Reserved0      = 0x00000040, // Always set to zero!
            LargeEntry     = 0x00000080, // 0 for 4 KiB pages, 1 for 4 MiB pages
            Global         = 0x00000100, // Prevents TLB invalidation

            SystemBit1     = 0x00000200, // For use by OS
            SystemBit2     = 0x00000400, // For use by OS
            SystemBit3     = 0x00000800, // For use by OS

            AddressMaskHi  = 0x00000000, // Address bits 32-39 

            Reserved1      = 0x00200000, // Reserved, set to zero

            AddressMaskLo  = 0xFFFFF000  // Address bits 22-31
        ;
    }

    /************************************************************************************************************/
    
    /// @brief Flags for page table entries.
    namespace PageEntryFlags
    {
        constexpr const uint32_t
            Present        = 0x00000001, // Set if this page is present.
            Writable       = 0x00000002, // Set if this is a writable page.
            User           = 0x00000004, // Set if this is a user space page.
            WriteThruCache = 0x00000008,
            DisableCache   = 0x00000010, // Set if cache is disabled
            Accessed       = 0x00000020, // Set by CPU if page has been accessed.
            Dirty          = 0x00000040, // Set by CPU if page has been written to.
            Reserved       = 0x00000080, // Always set to zero!
            Global         = 0x00000100,

            SystemBit1     = 0x00000200, // For use by OS
            SystemBit2     = 0x00000400, // For use by OS
            SystemBit3     = 0x00000800, // For use by OS

            AddressMask    = 0xFFFFF000  // Physical address (shifted right 12 bits)
        ;
    }

    /************************************************************************************************************/

#if 0
    constexpr uint32_t MapToDirFlags(PageFlags flags)
    {
        uint32_t rval = DirEntryFlags::User;

        if (flags && is_set(PageFlags::Write))
            rval |= DirEntryFlags::Writable;

        if (flags && is_set(PageFlags::Kernel))
            rval &= ~DirEntryFlags::User;

        return rval;
    }
#endif

    /************************************************************************************************************/

    constexpr uint32_t MapToPageFlags(PageFlags flags)
    {
        uint32_t rval = PageEntryFlags::User;

        if (flags && is_set(PageFlags::Write))
            rval |= PageEntryFlags::Writable;

        if (flags && is_set(PageFlags::Kernel))
            rval &= ~PageEntryFlags::User;

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

            if (!entry->Present)
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
            if (!pageTable->table[start].Present)
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
            if (!pageDir->tables[dirIndex].Present)
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
            if (!pageDir->tables[rval].Present)
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

        uintptr_t ptr = pde & DirEntryFlags::AddressMask;
        return reinterpret_cast<page_entry_t *>(PHYS_2_VIRT(ptr));
    }

    /************************************************************************************************************/
    /**
     * @brief Map a phyiscal memory page to a virtual memory page.
     * @remark Note that addresses and sizes might get aligned to processor page boundaries.
     * @param dir Directory to map the page in.
     * @param paddr The phyiscal address to be mapped
     * @param vaddr The virtual address
     * @param flags Flags to be set on the page (The Present flag is added automatically.)
     */
    Kernel::ErrorCode MapPage(paging::page_directory_t dir, paddr_t paddr, vaddr_t vaddr, PageFlags flags)
    {
        if (!IsAligned(paddr) || !IsAligned(vaddr))
            return Kernel::ErrorCode::NotAligned;

        //uint32_t dirFlags = MapToDirFlags(flags);
        uint32_t pageFlags = MapToPageFlags(flags);

        //Debug::PrintF("Map %p -> %p\r\n", paddr, vaddr);
        
        int pgtIndex = (vaddr >> 12) & 0x03FF;
        int dirIndex = (vaddr >> 22) & 0x03FF;

        page_directory_entry_t &pde = dir[dirIndex];

        // Assert these entries are correct!
        if ((pde & DirEntryFlags::Present) == 0)
            kpanic("Request to map to non present page entry!");

        page_entry_t entry = (paddr & PageEntryFlags::AddressMask) | pageFlags | PageEntryFlags::Present;

        page_entry_t *pageTable = GetPageTable(pde);
        page_entry_t &page = pageTable[pgtIndex];

        if ((page & PageEntryFlags::Present) != 0 && (entry != page))
            Debug::PrintF("WARNING: Page over writing: %p with %p\r\n", page, entry);

        page = entry;

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
        if ((pde & DirEntryFlags::Present) == 0)
            return Kernel::ErrorCode::NoError; // Nothing to do

        page_entry_t *pageTable = GetPageTable(pde);
        page_entry_t &page = pageTable[pgtIndex];

        if ((page & PageEntryFlags::Present) == 0)
            return Kernel::ErrorCode::NoError; // Nothing to do

        page &= ~PageEntryFlags::Present;

        return Kernel::ErrorCode::NoError;
    }

    /************************************************************************************************************/
    /**
     * @brief Get the physical page for the supplied virtual address.
     *
     * @param dir The page directory to look for the virtual address in.
     * @param vaddr The virtual address to lookup.
     *
     * @remarks The virtual address does not need to be aligned, but an aligned address will always be returned.
     *
     * @return An aligned physical address that is holds the supplied virtual address.  Or nullptr (zero) if not mapped.
     */
    paddr_t GetPhysicalPageFor(const paging::page_directory_t dir, vaddr_t vaddr)
    {
        int pgtIndex = (vaddr >> 12) & 0x03FF;
        int dirIndex = (vaddr >> 22) & 0x03FF;

        const page_directory_entry_t &pde = dir[dirIndex];

        // Assert these entries are correct!
        if ((pde & DirEntryFlags::Present) == 0)
            return 0; // Not mapped

        const page_entry_t *pageTable = GetPageTable(pde);
        const page_entry_t &page = pageTable[pgtIndex];
        
        if ((page & PageEntryFlags::Present) == 0)
            return 0; // Not mapped

        return reinterpret_cast<paddr_t>(page & PageEntryFlags::AddressMask);
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
        if (bootDir->tables[dirIndex].Present)
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
    page_index_t newPage = vmm::AllocRawPage();

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

paddr_t PageTable::doGetPhysicalPageFor(vaddr_t addr) const
{
    return ::GetPhysicalPageFor(m_dir, addr);
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
