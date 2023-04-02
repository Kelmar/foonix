/*************************************************************************/
/*************************************************************************/

//#include "assert.h"
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/vm/vm.h>

#include "cpu.h"

/*************************************************************************/

namespace
{
    // Compute space for a map of the lower 4MB of pages.
    const size_t LOWER_4MB        = 1024 * 1024 * 4;
    const size_t LOWER_4MB_BITS   = LOWER_4MB / PAGE_SIZE;
    const size_t LOWER_4MB_BYTES  = LOWER_4MB_BITS / 8;
    const size_t LOWER_4MB_DWORDS = LOWER_4MB_BYTES / sizeof(uint32_t);

    // Bitmap of lower 4MB of memory, we use this to bootstrap the rest of memory management.
    uint32_t s_LowerMap[LOWER_4MB_DWORDS];
}

/*************************************************************************/

using namespace VM;

/*************************************************************************/

void VM::Init(KernelArgs *ka)
{
    // Mark all pages free initially.
    memset(&s_LowerMap, 0, LOWER_4MB_BYTES);

    Arch::InitBootMemory(ka);

    Debug::PrintF("Memory Bitmap:\r\n");
    for (uint32_t i = 0; i < LOWER_4MB_DWORDS; i += 4)
        Debug::PrintF("    %08X %08X %08X %08X\r\n", s_LowerMap[i], s_LowerMap[i + 1], s_LowerMap[i + 2], s_LowerMap[i + 3]);
}

/*************************************************************************/

void VM::ReserveBootPages(addr_t physicalAddress, size_t count)
{
    //ASSERT(physicalAddress < LOWER_4MB, "Address too large!");

    size_t pageNum = physicalAddress / PAGE_SIZE;

    if ((pageNum + count) > 1024)
        count = 1024 - pageNum;

    for (size_t i = 0; i < count; ++i, ++pageNum)
    {
        size_t pageIndex = pageNum / (sizeof(uint32_t) * 8);
        size_t pageOffset = pageNum % (sizeof(uint32_t) * 8);

        s_LowerMap[pageIndex] |= 1 << pageOffset;
    }
}

/*************************************************************************/

void VM::ReservePage(addr_t start, size_t length)
{
    UNUSED(start);
    UNUSED(length);
}

/*************************************************************************/
