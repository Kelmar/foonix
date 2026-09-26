/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/boot_args.h>
#include <kernel/debug.h>
#include <kernel/vm.h>

#include "paging.h"
#include "x86priv.h"

using namespace paging;

/********************************************************************************************************************/

//typedef uint64_t page_entry_t;

//typedef page_table_t page_entry_t[512];

// Defined in start.S
extern "C" uintptr_t boot_pml4t;
extern "C" uintptr_t boot_pdpt;

/********************************************************************************************************************/

void paging::Preinit(boot::ArgumentData *argData)
{
    Debug::PrintF("ENTER: paging::Preinit()\r\n");

    Debug::PrintF("EXIT: paging::Preinit()\r\n");
}

/********************************************************************************************************************/

void paging::Init(memory::VPageMapBuilder &)
{
}

/********************************************************************************************************************/
