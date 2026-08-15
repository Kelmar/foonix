/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/bitmap.h>
#include <kernel/debug.h>

#include <kernel/vm.h>
#include <kernel/arch.h>

#include "asm.h"
#include "cpu.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "bootinfo.h"

#include "arch_vm.h"

#include "paging.h"

/********************************************************************************************************************/

namespace
{
    // Uses a simple bitmap allocation mark and free for the first 1MB of real memory pages.

    // Compute space for a map of the lower 1MB of pages.
    // We track these pages as special identity mapped pages.
    const size_t LOWER_1MB       = 1024 * 1024;
    const size_t LOWER_1MB_PAGES = LOWER_1MB / PAGE_SIZE;

    Bitmap<LOWER_1MB_PAGES> s_realMemMap;
}

/********************************************************************************************************************/
/*
 * Unconditionally marks a range of pages of real mode memory as in use.
 */
void Arch::VM::ReserveRealMemory(paddr_t addr, size_t length)
{
    if (addr >= LOWER_1MB)
        return;

    if ((addr + length) >= LOWER_1MB)
        length = LOWER_1MB - addr;

    // Make sure we start on a page boundary.
    uint64_t pageNumber = addr / PAGE_SIZE;

    // Tell the VM these pages have been reserved.
    size_t count = (length + (PAGE_SIZE - 1)) / PAGE_SIZE;

    for (size_t i = 0; i < count; ++i, ++pageNumber)
    {
        //s_realMemMap[pageNumber] = true; // Ideal
        s_realMemMap.Set(pageNumber);
    }
}

/********************************************************************************************************************/
/*
 * Allocates a page of real memory.
 *
 * A return of zero indicates that we weren't able to allocate a real memory page.
 *
 * Note that this function will never attempt to allocate the first page, leaving that reserved for a null pointer.
 *
 * To get at this page use the above ReserveRealMemory() function.
 */
paddr_t Arch::VM::AllocRealMemory(void)
{
    size_t index = 0; // See note about 0 index.

    // Find first free page.
    while (s_realMemMap[++index])
        ;

    if (index >= LOWER_1MB_PAGES)
        return 0; // No available memory!

    return (index * PAGE_SIZE);
}

/********************************************************************************************************************/

void Arch::VM::ReleaseRealMemory(paddr_t addr)
{
    if (addr == 0)
    {
        /*
         * We're not panicking here, as there could be a legitimate reason to pass nullptr into this function.
         *
         * This mimics the behaviour of free() and delete with a nullptr.
         *
         * Note that this also prevents the first page from ever being freed.  This is actually an ideal side
         * effect to ensure things don't try to use it, leaving it reserved indicating it's special status.
         */

        return; // Do not release the NULL page!
    }

    size_t index = (addr / PAGE_SIZE);

    //s_realMemMap[index] = false; // Ideal
    s_realMemMap.Clear(index);
}

/********************************************************************************************************************/
