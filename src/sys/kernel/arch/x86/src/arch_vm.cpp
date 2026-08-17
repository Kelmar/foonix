/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/vm.h>

#include "asm.h"
#include "cpu.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "bootinfo.h"

#include "paging.h"

/********************************************************************************************************************/

constexpr const uintptr_t Mark4MB = 4 * (1 << 20);

/********************************************************************************************************************/

// The assembly code will initialize these values for us.
uint32_t g_bootMagic;   /* Value from EAX register */
uint32_t g_bootInfoPtr; /* Value from EBX register */

/********************************************************************************************************************/
/*
 * These are defined in the linker script.
 */

/// @brief Physical memory location of the start of the kernel.
extern "C" uintptr_t _kernel_phys_start;

/// @brief Physical memory location of the end of the kernel.
extern "C" uintptr_t _kernel_end;

/********************************************************************************************************************/

void arch::Init(KernelArgs *ka)
{
    paging::Init(ka);

    Debug::PrintF("ENTER: Arch::Init()\r\n");

    Debug::PrintF("Boot Magic: 0x%08X\r\n", g_bootMagic);
   
    // Figure out where we live in physical memory.

    // _kernel_phys_start is already set at physical address
    uintptr_t kstart = reinterpret_cast<uintptr_t>(&_kernel_phys_start);

    // _kernel_end is set at virtual address.
    uintptr_t kend = reinterpret_cast<uintptr_t>(VIRT_2_PHYS(&_kernel_end));

    ka->KernelCode = MemoryRange::FromAddresses(kstart, kend);
    ka->HeapStart = reinterpret_cast<paddr_t>(paging::AlignCeiling(kend));

    Debug::PrintF("Kernel: %p 0x%08X\r\n", ka->KernelCode.Base, ka->KernelCode.Length);

    int err;

    switch (g_bootMagic)
    {
    case MULTIBOOT_MAGIC:
        err = Multiboot::ReadInfo(ka, g_bootInfoPtr);
        break;

    case MB2_MAGIC:
        err = MB2::ReadInfo(ka, g_bootInfoPtr);
        break;

    default:
        // TODO: Fallback to BIOS/EFI probe?
        err = -1;
        break;
    }

    if (err)
    {
        /*
         * We didn't get a memory map, we'll have to take a guess. For now we assume at end of kernel up to 4MB is okay.
         * Probably not the best solution, but it should be passible for testing.
         */
        Debug::PrintF("WARN: No memory map, guessing.\r\n");

        size_t len = Mark4MB - ka->HeapStart;
        ka->AddMemoryMap(ka->HeapStart, len);
    }
}

/********************************************************************************************************************/
