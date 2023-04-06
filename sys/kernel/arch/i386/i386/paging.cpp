/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel_args.h>
#include <kernel/arch/arch.h>
#include <kernel/arch/dconsole.h>
#include <kernel/debug.h>
#include <kernel/vm/vm.h>

#include "atomic.h"
#include "cpu.h"
#include "cpudefs.h"
#include "multiboot.h"
#include "page.h"
#include "arch_vm.h"

/********************************************************************************************************************/

using namespace Arch::Paging;

namespace
{
#if 0

    extern "C" Arch::Paging::page_directory_t boot_page_directory;
    extern "C" Arch::Paging::page_table_t boot_page_identity;
    extern "C" Arch::Paging::page_table_t boot_page_table1;

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
}

/********************************************************************************************************************/

using namespace Arch::Paging;

Kernel::ErrorCode Arch::InitPaging(KernelArgs *ka)
{
    UNUSED(ka);

#if 0

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
    
#endif
    return Kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/
