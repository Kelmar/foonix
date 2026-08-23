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

/********************************************************************************************************************/

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

/********************************************************************************************************************/

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

/********************************************************************************************************************/

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

/********************************************************************************************************************/
/********************************************************************************************************************/

/**
 * @brief Simple boot up allocator.
 *
 * @remarks This allocator cannot free pages.
 */
class BootPageAllocator : public IPageAllocator
{
private:
    BootInfo *m_bootInfo;

public:
    constexpr BootPageAllocator(BootInfo *bootInfo) noexcept
        : m_bootInfo(bootInfo)
    {
    }

    virtual ~BootPageAllocator() { }

    paddr_t AllocatePage() final
    {
        paddr_t rval = m_bootInfo->HeapNext;
        m_bootInfo->HeapNext += cpu::PageSize;
        return rval;
    }

    void ReleasePage(paddr_t) final
    {
        // Do nothing, we can't really free pages.
    }
};

/********************************************************************************************************************/
/********************************************************************************************************************/

/*
 * Nasty hack for now to pick which allocator to use.
 *
 * We can't use virtual methods or inheritance as the C++ compiler will look into the vtable to figure out which
 * method to call.  This method will have the pointer to where the function will be after we have the kernel running
 * in the higher half (after paging is setup), but not before.
 */
static bool g_useGlobalAlloc = false;

static char bpaBuffer[sizeof(BootPageAllocator)];
static BootPageAllocator *g_bootPageAllocator;

PageTable paging::g_bootPageTable;

/********************************************************************************************************************/
/********************************************************************************************************************/
/**
 * @brief Map PageFlags to X86 page directory entry flags.
 */
static
constexpr uint32_t MapToDirFlags(PageFlags flags)
{
    uint32_t rval = 0;

    if (has_any(flags, PageFlags::Write | PageFlags::Execute))
        rval |= DirEntryFlags::Writable;

    if (!is_set(flags, PageFlags::Kernel))
        rval |= DirEntryFlags::User;

    return rval;
}

/********************************************************************************************************************/
/**
 * @brief Map PageFlags to X86 page table entry flags.
 */
static
constexpr uint32_t MapToEntryFlags(PageFlags flags)
{
    uint32_t rval = 0;

    if (has_any(flags, PageFlags::Write | PageFlags::Execute))
        rval |= PageEntryFlags::Writable;

    if (!is_set(flags, PageFlags::Kernel))
        rval |= PageEntryFlags::User;

    return rval;
}

/********************************************************************************************************************/

PageTable::PageTable() noexcept
    : PageTableBase()
    , m_dir(0)
{
    m_dir = reinterpret_cast<page_directory_entry_t *>(AllocatePage());
    memset(m_dir, 0, sizeof(page_directory_t));
}

PageTable::PageTable(page_directory_entry_t *directory, DirectoryOptions options /* = DirectoryOptions::None */) noexcept
    : PageTableBase()
    , m_dir(directory)
{
    if (!is_set(options, DirectoryOptions::NoClear))
        memset(m_dir, 0, sizeof(page_directory_t));
}

/********************************************************************************************************************/

paddr_t PageTable::AllocatePage()
{
    paddr_t rval = g_useGlobalAlloc ?
        page_allocator.AllocatePage() :
        g_bootPageAllocator->AllocatePage();

    //Debug::PrintF("Allocated page @%p\r\n", rval);

    return rval;
}

/********************************************************************************************************************/
/**
 * @brief Add a page table to a directory.
 *
 * @param dir The directory to add the page table to.
 * @param index The directory index to add the page table to.
 * @param table The physical address of the page table to add.
 * @param PageFlags Flags for mapping the table into the directory.
 */
Kernel::ErrorCode PageTable::AddDirectoryEntry(size_t index, paddr_t table, PageFlags flags)
{
    //DEBUG_ASSERT(index < TableEntries, "Invalid directory index for AddDirectoryEntry() call.");
    //DEBUG_ASSERT((m_dir[index] & DirEntryFlags::Preset) == 0, "Request to map page table to already mapped directory entry.");

    void *ptr = reinterpret_cast<void *>(table);
    memset(ptr, 0, sizeof(page_table_t));

    uint32_t dirFlags = MapToDirFlags(flags);
    
    dirFlags |= PageEntryFlags::Present;

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
    return reinterpret_cast<page_entry_t *>(paddr); //PHYS_2_VIRT(paddr));
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
        paddr_t paddr = AllocatePage();

        result = reinterpret_cast<page_entry_t *>(paddr); //PHYS_2_VIRT(paddr)); // TODO: Needs mapping fix.
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

    uint32_t pageFlags = MapToEntryFlags(flags) | PageEntryFlags::Present;
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

static
void InitBootPages(BootInfo *bootInfo, PageTable *bootPageTable)
{
    paddr_t kstart = paging::AlignFloor(bootInfo->KernelStart);
    paddr_t klast = paging::AlignCeiling(bootInfo->KernelEnd);

    // Start at 1, leaving 0 as "not present" for nullptr dereference checks.

    for (size_t i = 1; i < TableEntries; ++i)
    {
        paddr_t paddr = reinterpret_cast<paddr_t>(i * cpu::PageSize);
        vaddr_t vaddr = reinterpret_cast<vaddr_t>(paddr); // 1:1 mapping for identity.

        bootPageTable->MapPage(paddr, vaddr, PageFlags::Write | PageFlags::Kernel);

        if ((paddr >= kstart) && (paddr <= klast))
        {
            // Set higher half value.
            vaddr = PHYS_2_VIRT(paddr);
            bootPageTable->MapPage(paddr, vaddr, PageFlags::Execute | PageFlags::Kernel);
        }
    }
}

/********************************************************************************************************************/

void paging::Init(KernelArgs *)
{
    //new (&g_bootPageTable) PageTable(boot_page_directory, DirectoryOptions::NoClear);
    g_useGlobalAlloc = true;
}

void paging::Preinit(BootInfo *bootInfo)
{
    Debug::PrintF("ENTER: paging::Preinit()\r\n");

    g_bootPageAllocator = reinterpret_cast<BootPageAllocator *>(bpaBuffer);
    PageTable *bootPageTable = &g_bootPageTable;

    new (g_bootPageAllocator) BootPageAllocator(bootInfo);
    new (bootPageTable) PageTable();

    InitBootPages(bootInfo, bootPageTable);

    bootPageTable->MakeActive();

    Debug::PrintF("EXIT: paging::Preinit()\r\n");
}

/********************************************************************************************************************/
