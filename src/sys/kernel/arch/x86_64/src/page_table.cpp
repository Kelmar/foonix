/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kassert.h>

#include <kernel/boot_args.h>
#include <kernel/console.h>
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
        LargeEntry     = 0x0000'0000'0000'0080, // Not always supported above 2MB pages.
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

/********************************************************************************************************************/
#if 0

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
    : m_root(nullptr)
{
    m_root = reinterpret_cast<uint64_t *>(AllocatePage());
}

PageTable::PageTable(uint64_t *root) noexcept
    : m_root(root)
{
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
    size_t shift = EntryShift(MaxLevels - 1);
    uint64_t *table = m_root;

    for (size_t i = MaxLevels - 1; i > 0; --i, shift -= ShiftInc)
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
 * @brief Map a physical memory page to a virtual memory page.
 * @remark Note that addresses and sizes might get aligned to processor page boundaries.
 * @param paddr The physical address to be mapped
 * @param vaddr The virtual address
 * @param flags Flags to be set on the page (The Present flag is added automatically.)
 */
kernel::ErrorCode PageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    uint64_t entryFlags = MapToEntryFlags(flags) | EntryFlags::Present;

    MapSinglePage(paddr, vaddr, entryFlags);

    return kernel::ErrorCode::NoError;
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

void PageTable::PrintState(VisitState *state) const
{
    if (!state->inSpan)
        return;

    console << "0x" << ptrfmt(state->start) << " : " << szfmt(state->size) << " bytes\r\n";
}

/********************************************************************************************************************/

void PageTable::VisitLevel(VisitState *state, const void *ptr, int depth) const
{
    size_t entrySize = cpu::PageSize * util::ipow(EntryCount, depth);
    const uint64_t *table = reinterpret_cast<const uint64_t *>(ptr);
    uintptr_t offset = state->offset;

    for (size_t i = 0; i < EntryCount; ++i)
    {
        const uint64_t &entry = table[i];

        state->offset = offset + (i * entrySize);

        if (state->offset & 0x0000'8000'0000'000)
            state->offset |= 0xFFFF'8000'0000'0000; // The 48th bit is sign extended.

        if ((entry & EntryFlags::Present) == 0)
        {
            PrintState(state);
            state->inSpan = false;

            continue;
        }

        if ((depth == 0) || (entry & EntryFlags::LargeEntry) != 0)
        {
            if (state->inSpan)
                state->size += entrySize;
            else
            {
                state->inSpan = true;
                state->start = state->offset;
                state->size = entrySize;
            }

            continue;
        }

        // Address points to a sub table.
        
        ptr = reinterpret_cast<const void *>(entry & EntryAddressMask);
        VisitLevel(state, ptr, depth - 1);
    }
}

/********************************************************************************************************************/

void PageTable::doDump() const
{
    VisitState state;

    memset(&state, 0, sizeof(state));

    VisitLevel(&state, m_root, MaxLevels - 1);

    PrintState(&state);
}

/********************************************************************************************************************/
