/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X64_PAGING_H__
#define __FOONIX_KERNEL_ARCH_X64_PAGING_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

//#include <kernel/kernel.h>
//#include <kernel/utilities.h>

//#include <kernel/vm.h> // We can't include this here, vm.h needs to reference us.

#include "cpu.h"

/********************************************************************************************************************/

namespace memory
{
    class VPageMapBuilder;
}

/********************************************************************************************************************/

namespace paging
{
    /************************************************************************************************************/

    constexpr const size_t EntryCount = 512;

    /************************************************************************************************************/

    class PageTable : public PageTableBase<PageTable>
    {
    private:
        /// @brief The maximum number of page levels supported.
        static constexpr size_t MaxLevels = 4;

        /// @brief Number of entries per table.
        static constexpr size_t TableEntries = 512; // (4KB / sizeof(uint64_t))

        /// @brief Mask to get index from virtual address after shifting.
        static constexpr uint64_t EntityIndexMask = (TableEntries - 1);

        /// @brief Mask for stripping off additional address bits for an entry.
        static constexpr uint64_t EntryAddressMask = 0x000F'FFFF'FFFF'F000;

        /// @brief Base number we start shifting by.
        static constexpr size_t ShiftBase = 12;

        /// @brief Number of bits we increase our shift by for each level of paging.
        static constexpr size_t ShiftInc = 9;

        struct VisitState
        {
            uintptr_t offset; // Offset start for each table we're visiting (updated each call to VisitLevel())

            bool inSpan;
            uintptr_t start;
            size_t size;
        };

    private:
        uint64_t *m_root;

        paddr_t AllocatePage();

        /**
         * @brief Compute shift based on the requested table level.
         *
         * @details Computes the amount of shifting based on what level of table's were computing for.
         * With levels being:
         * 0 - 4K page table entry
         * 1 - 2MB directory table entry
         * 2 - 1GB page directory pointer table entry
         * 3 - Level 4 page map entry
         * 4 - Level 5 page map entry
         */
        static
        uint64_t EntryShift(size_t level) { return (ShiftBase + (ShiftInc * level)); }

        static
        uint64_t MapToEntryFlags(PageFlags flags);

        void MapSinglePage(paddr_t paddr, vaddr_t vaddr, uint64_t entryFlags);

        void PrintState(VisitState *state) const;
        void VisitLevel(VisitState *state, const void *table, int depth) const;

    public:
        PageTable() noexcept;
        PageTable(uint64_t *root) noexcept;

        virtual ~PageTable() { }

        kernel::ErrorCode doMapPage(paddr_t paddr, vaddr_t vaddr, PageFlags flags);
        kernel::ErrorCode doUnmapPage(vaddr_t vaddr);

        paddr_t doGetPhysicalPageFor(vaddr_t vaddr) const;

        void doMakeActive() const { x86::load_cr3(reinterpret_cast<uintptr_t>(m_root)); }

        void doDump() const;
    };

    extern PageTable g_bootPageTable;

    /************************************************************************************************************/

    void Init(memory::VPageMapBuilder &);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X64_PAGING_H__ */

/********************************************************************************************************************/
