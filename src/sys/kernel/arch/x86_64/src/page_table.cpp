/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>

#include "paging.h"

using namespace paging;

/********************************************************************************************************************/

namespace
{
    /************************************************************************************************************/

    /// @brief Flags for Page Directory Pointer Table entries
    namespace PDPTEntryFlags_1GB
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

            AttributeTable = 0x0000'0000'0000'1000,

            Reserved       = 0x0000'0000'3FFF'E000, // Bits [13-29]

            /*
             * Capped by the limits of the specific CPU.  Earlier models
             * did not have all address lines enabled; those upper bits
             * must remain 0 if that is the case.
             */
            AddressMask    = 0x000F'FFFF'C000'0000,

            // Speculate these might not actually be available to us.
            SystemBit4     = 0x0010'0000'0000'0000, // OS defined bit 4
            SystemBit5     = 0x0020'0000'0000'0000, // OS defined bit 5
            SystemBit6     = 0x0040'0000'0000'0000, // OS defined bit 6
            SystemBit7     = 0x0080'0000'0000'0000, // OS defined bit 7
            SystemBit8     = 0x0100'0000'0000'0000, // OS defined bit 8
            SystemBit9     = 0x0200'0000'0000'0000, // OS defined bit 9
            SystemBit10    = 0x0400'0000'0000'0000, // OS defined bit 10

            ProtectKey     = 0x7800'0000'0000'0000,
            ExecDisable    = 0x8000'0000'0000'0000
        ;
    }

    /************************************************************************************************************/

    /// @brief Page directory entry flags
    namespace DirectoryEntryFlags_2MB
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

            SystemBit1     = 0x0000'0000'0000'0200, // For use by OS
            SystemBit2     = 0x0000'0000'0000'0400, // For use by OS
            SystemBit3     = 0x0000'0000'0000'0800, // For use by OS

            AttributeTable = 0x0000'0000'0000'1000,

            Reserved       = 0x0000'0000'001F'E000, // Bits [13-29]

            // Like PDPT above, unsupported bits must be zero?
            AddressMask    = 0x000F'FFFF'FFE0'0000,

            // Speculate these might not actually be available to us.
            SystemBit4     = 0x0010'0000'0000'0000, // OS defined bit 4
            SystemBit5     = 0x0020'0000'0000'0000, // OS defined bit 5
            SystemBit6     = 0x0040'0000'0000'0000, // OS defined bit 6
            SystemBit7     = 0x0080'0000'0000'0000, // OS defined bit 7
            SystemBit8     = 0x0100'0000'0000'0000, // OS defined bit 8
            SystemBit9     = 0x0200'0000'0000'0000, // OS defined bit 9
            SystemBit10    = 0x0400'0000'0000'0000, // OS defined bit 10

            ProtectKey     = 0x7800'0000'0000'0000,
            ExecDisable    = 0x8000'0000'0000'0000
        ;
    }

    /************************************************************************************************************/

    /// @brief Page table entry flags
    namespace PageTableEntryFlags
    {
        constexpr const uint64_t
            Present        = 0x0000'0000'0000'0001,
            Writable       = 0x0000'0000'0000'0002,
            User           = 0x0000'0000'0000'0004,
            WriteThruCache = 0x0000'0000'0000'0008,
            DisableCache   = 0x0000'0000'0000'0010,
            Accessed       = 0x0000'0000'0000'0020, // Set by CPU
            Dirty          = 0x0000'0000'0000'0040, // Set by CPU
            AttributeTable = 0x0000'0000'0000'0080,
            Global         = 0x0000'0000'0000'0100,

            SystemBit1     = 0x0000'0000'0000'0200, // For use by OS
            SystemBit2     = 0x0000'0000'0000'0400, // For use by OS
            SystemBit3     = 0x0000'0000'0000'0800, // For use by OS

            // Like PDPT & PDT above, unsupported bits must be zero?
            AddressMask    = 0x000F'FFFF'FFFF'F000,

            // Speculate these might not actually be available to us.
            SystemBit4     = 0x0010'0000'0000'0000, // OS defined bit 4
            SystemBit5     = 0x0020'0000'0000'0000, // OS defined bit 5
            SystemBit6     = 0x0040'0000'0000'0000, // OS defined bit 6
            SystemBit7     = 0x0080'0000'0000'0000, // OS defined bit 7
            SystemBit8     = 0x0100'0000'0000'0000, // OS defined bit 8
            SystemBit9     = 0x0200'0000'0000'0000, // OS defined bit 9
            SystemBit10    = 0x0400'0000'0000'0000, // OS defined bit 10

            ProtectKey     = 0x7800'0000'0000'0000,
            ExecDisable    = 0x8000'0000'0000'0000
        ;
    }

    /************************************************************************************************************/
    /**
     * @brief Map a phyiscal memory page to a virtual memory page.
     * @remark Note that addresses and sizes might get aligned to processor page boundaries.
     * @param pdpt ???? to map the page in.
     * @param paddr The phyiscal address to be mapped
     * @param vaddr The virtual address
     * @param flags Flags to be set on the page (The Present flag is added automatically.)
     */
    Kernel::ErrorCode MapPage(pdpt_t pdpt, paddr_t paddr, vaddr_t vaddr, PageFlags flags)
    {
        return Kernel::ErrorCode::Unknown;
    }

    /************************************************************************************************************/
    /**
     * @brief Remove a virtual page from paging.
     * @param pdpt ???? to unmap from.
     * @param vaddr The virtual address to unmap.
     */
    Kernel::ErrorCode UnmapPage(pdpt_t pdpt, vaddr_t vaddr)
    {
        return Kernel::ErrorCode::Unknown;
    }

    /************************************************************************************************************/
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
    paddr_t GetPhysicalPageFor(const pdpt_t pdpt, vaddr_t vaddr)
    {
        return 0;
    }

    /************************************************************************************************************/
}

/********************************************************************************************************************/

PageTable::PageTable()
{
}

/********************************************************************************************************************/

Kernel::ErrorCode PageTable::doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags)
{
    return ::MapPage(m_pdpt, paddr, vaddr, flags);
}

/********************************************************************************************************************/

Kernel::ErrorCode PageTable::doUnmapPage(vaddr_t vaddr)
{
    return ::UnmapPage(m_pdpt, vaddr);
}

/********************************************************************************************************************/

paddr_t PageTable::doGetPhysicalPageFor(vaddr_t vaddr) const
{
    return ::GetPhysicalPageFor(m_pdpt, vaddr);
}

/********************************************************************************************************************/
