/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/vm.h>

#include "cpu.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "bootinfo.h"

/********************************************************************************************************************/

// The assembly code will initialize these values for us.
uint32_t g_BootMagic; /* Value from EAX register */
uint32_t g_Multiboot; /* Value from EBX register */

/********************************************************************************************************************/
/*
 * These are defined in the linker script.
 */

/// @brief Physical memory location of the start of the kernel.
extern "C" uintptr_t _kernel_phys_start;

/// @brief Physical memory location of the end of the kernel.
extern "C" uintptr_t _kernel_end;

constexpr void *kernel_start = &_kernel_phys_start;

constexpr void *kernel_end = &_kernel_end; //VIRT_2_PHYS(&_kernel_end);

/********************************************************************************************************************/

void arch::InitBootMemory(KernelArgs *ka)
{
    Debug::PrintF("ENTER: Arch::InitBootMemory()\r\n");

    Debug::PrintF("Boot Magic: 0x%08X\r\n", g_BootMagic);
    
    // Figure out where we live in physical memory.
    ka->KernelCode.Base = reinterpret_cast<uintptr_t>(&kernel_start);
    uintptr_t kend = reinterpret_cast<uintptr_t>(&kernel_end);

    ka->KernelCode.Length = kend - ka->KernelCode.Base;

    Debug::PrintF("Kernel: %p 0x%08X\r\n", ka->KernelCode.Base, ka->KernelCode.Length);

    int err;

    switch (g_BootMagic)
    {
#if 0
    case MULTIBOOT_MAGIC:
        err = Multiboot::ReadInfo(ka, g_Multiboot);
        break;
#endif

    case MB2_MAGIC:
        err = MB2::ReadInfo(ka, g_Multiboot);
        break;

    default:
        // TODO: Fallback to BIOS/EFI probe?
        err = -1;
        break;
    }

    if (err)
        Debug::PrintF("WARN: No memory map, guessing.\r\n");
}

/********************************************************************************************************************/
