/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/boot_args.h>
#include <kernel/vm.h>

#include "paging.h"

using namespace paging;

/********************************************************************************************************************/

//typedef uint64_t page_entry_t;

//typedef page_table_t page_entry_t[512];

// Defined in start.S
extern "C" uintptr_t boot_pml4t;
extern "C" uintptr_t boot_pdpt;
extern "C" uintptr_t boot_pdt;
extern "C" uintptr_t boot_pt;

/********************************************************************************************************************/

void paging::Init(memory::VPageMapBuilder &)
{
}

/********************************************************************************************************************/
