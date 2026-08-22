/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel_args.h>
#include <kernel/debug.h>

#include <kernel/arch.h>

#include <kernel/vm.h>
#include <kernel/vm/new.h>
#include <kernel/vm/page_table.h>

#include "bootinfo.h"

#include "asm.h"
#include "atomic.h"
#include "cpu.h"
#include "cpudefs.h"
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
        static constexpr const uint32_t
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
        static constexpr const uint32_t
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
        static constexpr const uint32_t
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

    constexpr uint32_t MapToDirFlags(PageFlags flags)
    {
        uint32_t rval = DirEntryFlags::User;

        if (flags && is_set(PageFlags::Write))
            rval |= DirEntryFlags::Writable;

        if (flags && is_set(PageFlags::Kernel))
            rval &= ~DirEntryFlags::User;

        return rval;
    }

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

    
}

/********************************************************************************************************************/

// Defined in start.S
extern "C" page_directory_t boot_page_directory;
extern "C" page_table_t boot_page_identity;
extern "C" page_table_t boot_page_kernel;

PageTable paging::g_bootPageTable;

static page_directory_t boot_page_directory_new;

PageTable paging::g_bootPageTableNew;

/********************************************************************************************************************/
/********************************************************************************************************************/

/*
PageTable::PageTable()
    : PageTableBase()
{
    memset(m_dir, 0, sizeof(page_directory_t));
}
*/

PageTable::PageTable(page_directory_entry_t *directory, DirectoryOptions options /* = DirectoryOptions::None */)
    : PageTableBase()
    , m_dir(directory)
{
    if (!(options && is_set(DirectoryOptions::NoClear)))
    {
        memset(m_dir, 0, sizeof(page_directory_t));
    }
}

/********************************************************************************************************************/

Kernel::ErrorCode PageTable::AddDirectoryEntry(size_t index, paddr_t table, PageFlags flags)
{
    //DEBUG_ASSERT(index < TableEntries, "Invalid directory index for AddDirectoryEntry() call.");
    //DEBUG_ASSERT((m_dir[index] & DirEntryFlags::Preset) == 0, "Request to map page table to already mapped directory entry.");

    void *ptr = reinterpret_cast<void *>(table);
    memset(ptr, 0, sizeof(page_table_t));

    uint32_t dirFlags = MapToDirFlags(flags) | PageEntryFlags::Present;

    m_dir[index] = (table & PageEntryFlags::AddressMask) | dirFlags;
    return Kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/
/**
 * @brief Locate a table entry in the directory.  If none is found, then nullptr is returned.
 */
page_entry_t *PageTable::GetPageTable(vaddr_t vaddr, size_t &dirIndex) const
{
    dirIndex = ToDirIndex(vaddr);

    page_directory_entry_t &pde = m_dir[dirIndex];

    if ((pde & DirEntryFlags::Present) == 0)
        return nullptr;

    /*
     * TODO: The pointer in the directory entry is the physical address, find a good
     * way we can map that into the kernel space so we can update it.
     *
     * Right now we have it hard coded to our PHYS_2_VIRT macro.
     */
    paddr_t paddr = pde & DirEntryFlags::AddressMask;
    return reinterpret_cast<page_entry_t *>(PHYS_2_VIRT(paddr));
}

/********************************************************************************************************************/
/**
 * @brief Locates a table entry in the directory, if none is found, a new one is allocated.
 */
page_entry_t *PageTable::GetOrCreatePageTable(vaddr_t vaddr, PageFlags flags)
{
    size_t dirIndex;

    page_entry_t *result = GetPageTable(vaddr, dirIndex);

    if (result == nullptr)
    {    
        paddr_t paddr = page_allocator.AllocatePage();
        result = reinterpret_cast<page_entry_t *>(PHYS_2_VIRT(paddr)); // TODO: Needs mapping fix.
        AddDirectoryEntry(dirIndex, paddr, flags);
    }

    return result;
}

/********************************************************************************************************************/
/**
 * @brief Map a phyiscal memory page to a virtual memory page.
 * @remark Note that addresses and sizes might get aligned to processor page boundaries.
 * @param dir Directory to map the page in.
 * @param paddr The phyiscal address to be mapped
 * @param vaddr The virtual address
 * @param flags Flags to be set on the page (The Present flag is added automatically.)
 */
Kernel::ErrorCode PageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    if (!IsAligned(paddr) || !IsAligned(vaddr))
        return Kernel::ErrorCode::NotAligned;

    //Debug::PrintF("Map %p -> %p\r\n", paddr, vaddr);

    page_entry_t *pageTable = GetOrCreatePageTable(vaddr, flags);

    uint32_t pageFlags = MapToPageFlags(flags) | PageEntryFlags::Present;
    page_entry_t entry = (paddr & PageEntryFlags::AddressMask) | pageFlags;
    
    size_t pgtIndex = ToEntryIndex(vaddr);
    page_entry_t &page = pageTable[pgtIndex];

    if ((page & PageEntryFlags::Present) != 0 && (entry != page))
        Debug::PrintF("WARNING: Page over writing: %p with %p\r\n", page, entry);

    page = entry;

    return Kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/
/**
 * @brief Remove a virtual page from paging.
 * @param dir Directory to unmap from.
 * @param vaddr The virtual address to unmap.
 */
Kernel::ErrorCode PageTable::doUnmapPage(vaddr_t vaddr)
{
    if (!IsAligned(vaddr))
        return Kernel::ErrorCode::NotAligned;
    
    int pgtIndex = (vaddr >> 12) & 0x03FF;
    int dirIndex = (vaddr >> 22) & 0x03FF;

    page_directory_entry_t &pde = m_dir[dirIndex];

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

/********************************************************************************************************************/
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
paddr_t PageTable::doGetPhysicalPageFor(vaddr_t vaddr) const
{
    int pgtIndex = (vaddr >> 12) & 0x03FF;
    int dirIndex = (vaddr >> 22) & 0x03FF;

    const page_directory_entry_t &pde = m_dir[dirIndex];

    // Assert these entries are correct!
    if ((pde & DirEntryFlags::Present) == 0)
        return 0; // Not mapped

    const page_entry_t *pageTable = GetPageTable(pde);
    const page_entry_t &page = pageTable[pgtIndex];
    
    if ((page & PageEntryFlags::Present) == 0)
        return 0; // Not mapped

    return reinterpret_cast<paddr_t>(page & PageEntryFlags::AddressMask);
}

/********************************************************************************************************************/
/********************************************************************************************************************/

#if 0

static
void InitIdentityPages(BootInfo *bootInfo)
{
    for (size_t i = 0; i < TableEntries; ++i)
    {
        paddr_t paddr = reinterpret_cast<paddr_t>(i * cpu::PageSize);
        vaddr_t vaddr = reinterpret_cast<vaddr_t>(paddr); // 1:1 mapping for identity.

        g_bootPageTableNew.MapPage(paddr, vaddr, PageFlags::Write);
    }
}

static
void InitKernelPages(BootInfo *bootInfo)
{
    for (size_t i = 0; i < TableEntries; ++i)
    {
        paddr_t paddr = reinterpret_cast<paddr_t>(i * cpu::PageSize);

        bool inRange = (paddr >= g_kernelArguments.KernelCode.BaseAligned()) &&
            (paddr <= g_kernelArguments.KernelCode.EndAligned());

        if (inRange)
        {
            vaddr_t vaddr = PHYS_2_VIRT(paddr);
            g_bootPageTableNew.MapPage(paddr, vaddr, PageFlags::Execute | PageFlags::Kernel);
        }
    }
}

#endif

/********************************************************************************************************************/

void paging::Init(KernelArgs *)
{
    new (&g_bootPageTable) PageTable(boot_page_directory, DirectoryOptions::NoClear);
}

void paging::Preinit(BootInfo *bootInfo)
{
    PageTable *bootPageTable = &g_bootPageTableNew;
    //bootInfo->BootPageTable = bootPageTable;

    new (bootPageTable) PageTable(boot_page_directory_new);

    //InitIdentityPages(bootInfo);
    //InitKernelPages(bootInfo);
}

/********************************************************************************************************************/
