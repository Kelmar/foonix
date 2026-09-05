/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>

#include "paging.h"

#include <kernel/kernel.h>

#include <kernel/vm/vpage_map.h>

/********************************************************************************************************************/

//typedef uint64_t page_entry_t;

//typedef page_table_t page_entry_t[512];

// Defined in start.S
extern "C" uintptr_t boot_pml4t;
extern "C" uintptr_t boot_pdpt;
extern "C" uintptr_t boot_pdt;
extern "C" uintptr_t boot_pt;

/********************************************************************************************************************/

kernel::ErrorCode paging::Init(memory::VPageMapBuilder &)
{
    return kernel::ErrorCode::NoError;
}

/********************************************************************************************************************/
