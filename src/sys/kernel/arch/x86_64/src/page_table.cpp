/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kassert.h>

#include <kernel/boot_args.h>
#include <kernel/vm.h>

#include "paging.h"

using namespace paging;

/********************************************************************************************************************/
/**
 * @brief A list of flags that we know of.
 * @remarks There are others, leave those off for now.
 */
namespace EntryFlags
{
    constexpr const uint64_t
        Present        = 0x0000'0000'0000'0001,
        Writable       = 0x0000'0000'0000'0002,
        User           = 0x0000'0000'0000'0004,
        WriteThruCache = 0x0000'0000'0000'0008,
        DisableCache   = 0x0000'0000'0000'0010,
        Accessed       = 0x0000'0000'0000'0020, // Set by CPU
        Dirty          = 0x0000'0000'0000'0040, // Set by CPU
        LargeEntry     = 0x0000'0000'0000'0080,
        Global         = 0x0000'0000'0000'0100,

        SystemBit1     = 0x0000'0000'0000'0200, // OS defined bit 1
        SystemBit2     = 0x0000'0000'0000'0400, // OS defined bit 2
        SystemBit3     = 0x0000'0000'0000'0800, // OS defined bit 3

        /*
         * Note that this bit spills into the address, it is only needed if you use the large pages.
         */
         //Attribute    = 0x0000'0000'0000'1000,

        ExecDisable     = 0x8000'0000'0000'0000
    ;
}

/// @brief Number of entries per table.
constexpr size_t TableEntries = 512; // (4KB / sizeof(uint64_t))

constexpr uint64_t EntityIndexMask = (TableEntries - 1);

/// @brief Mask for stripping off additional address bits for an entry.
constexpr uint64_t EntryAddressMask = 0x000F'FFFF'FFFF'F000;

/// @brief Base number we start shifting by.
constexpr size_t ShiftBase = 12;

/// @brief Number of bits we increase our shift by for each level of paging.
constexpr size_t ShiftInc = 9;

/********************************************************************************************************************/
#if 0

/**
 * @brief Compute shift based on table level we're at
 *
 * @details Computes the amount of shifting based on what level of table's were computing for.
 * With levels being:
 * 0 - 4K page table entry
 * 1 - 2MB directory table entry
 * 2 - 1GB page directory pointer table entry
 * 3 - Level 4 page map entry
 * 4 - Level 5 page map entry.
 */
static inline
constexpr uint64_t EntryShift(size_t level)
{
    return (ShiftBase + (ShiftInc * level));
}

/********************************************************************************************************************/

/// @brief Build an entry for a given physical address, entry flags, and shift level.
static inline
constexpr uint64_t MakeEntry(paddr_t paddr, uint64_t flags, size_t shift)
{
    //return ((paddr >> shift) & EntryAddressMask) << shift | flags;
    return 0; // Need to rethink this.
}
#endif

/********************************************************************************************************************/
/********************************************************************************************************************/

PageTable::PageTable() noexcept
    : m_topLevel(nullptr)
{
    m_topLevel = reinterpret_cast<uint64_t *>(AllocatePage());
}

/********************************************************************************************************************/

paddr_t PageTable::AllocatePage()
{
    return 0;
}

/********************************************************************************************************************/

uint64_t PageTable::MapToEntryFlags(PageFlags flags)
{
    uint64_t result = 0;

    if (has_any(flags, PageFlags::Write | PageFlags::Execute))
        result |= EntryFlags::Writable;

    if (!is_set(flags, PageFlags::Kernel))
        result |= EntryFlags::User;

    return result;
}

/********************************************************************************************************************/

void PageTable::MapSinglePage(paddr_t paddr, vaddr_t vaddr, uint64_t entityFlags)
{
    size_t shift = ShiftBase + (MaxLevels - 2) * ShiftInc;
    uint64_t *table = m_topLevel;

    for (size_t i = MaxLevels - 1; i > 1; --i, shift -= ShiftInc)
    {
        size_t index = (vaddr >> shift) & EntityIndexMask;
        uint64_t &entry = table[index];

        if ((entry & EntryFlags::Present) == 0)
        {
            paddr_t newPage = AllocatePage();
            ASSERT(newPage, "Unable to allocate page!");

            entry = (newPage & EntryAddressMask) | entityFlags;
        }

        // Shift to the next level of pages.
        table = reinterpret_cast<uint64_t *>(entry & EntryAddressMask);
    }

    size_t index = (vaddr >> shift) & EntityIndexMask;
    uint64_t &entry = table[index];

    entry = (paddr & EntryAddressMask) | entityFlags;
}

/********************************************************************************************************************/
/**
 * @brief Map a phyiscal memory page to a virtual memory page.
 * @remark Note that addresses and sizes might get aligned to processor page boundaries.
 * @param paddr The phyiscal address to be mapped
 * @param vaddr The virtual address
 * @param flags Flags to be set on the page (The Present flag is added automatically.)
 */
kernel::ErrorCode PageTable::doMapPages(paddr_t paddr, vaddr_t vaddr, size_t count, PageFlags flags)
{
    uint64_t entryFlags = MapToEntryFlags(flags) | EntryFlags::Present;

    MapSinglePage(paddr, vaddr, entryFlags);

    return kernel::ErrorCode::Unknown;
}

/********************************************************************************************************************/

kernel::ErrorCode PageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    return kernel::ErrorCode::Unknown;
}

/********************************************************************************************************************/
/**
 * @brief Remove a virtual page from paging.
 * @param vaddr The virtual address to unmap.
 */
kernel::ErrorCode PageTable::doUnmapPage(vaddr_t vaddr)
{
    return kernel::ErrorCode::Unknown;
}

/********************************************************************************************************************/
/**
 * @brief Get the physical page for the supplied virtual address.
 *
 * @param pdpt The ???? to look for the virtual address in.
 * @param vaddr The virtual address to lookup.
 *
 * @remarks The virtual address does not need to be aligned, but an aligned address will always be returned.
 *
 * @return An aligned physical address that is holds the supplied virtual address.  Or nullptr (zero) if not mapped.
 */
paddr_t PageTable::doGetPhysicalPageFor(vaddr_t vaddr) const
{
    return 0;
}

/********************************************************************************************************************/
