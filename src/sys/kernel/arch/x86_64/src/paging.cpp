/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/boot_args.h>
#include <kernel/debug.h>
#include <kernel/vm.h>

#include <kernel/kalloc.h>

#include "paging.h"
#include "x86priv.h"

using namespace paging;

/********************************************************************************************************************/

// Kernel top level page table, defined and aligned in start.S
extern "C" uint64_t g_kernelTable[EntryCount];

uint32_t g_bootAllocatedPages = 0;

PageTable paging::g_bootPageTable;

/********************************************************************************************************************/

void paging::Preinit(boot::ArgumentData *argData)
{
    Debug::PrintF("ENTER: paging::Preinit()\r\n");

    Debug::PrintF("EXIT: paging::Preinit()\r\n");

    new (&paging::g_bootPageTable) PageTable(g_kernelTable);
}

/********************************************************************************************************************/

void paging::Init(memory::VPageMapBuilder &)
{
}

/********************************************************************************************************************/
