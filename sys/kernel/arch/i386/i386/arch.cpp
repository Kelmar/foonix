/*************************************************************************/
/*************************************************************************/

#include <stdint.h>

#include <kernel/arch/arch.h>
#include <kernel/kernel_args.h>
#include <kernel/debug.h>

#include "cpu.h"
#include "multiboot.h"

/*************************************************************************/

enum BootMethod : uint32_t
{
    /// @brief Booted with an unknown bootloader, expect wild wild west.
    BM_Unknown   = 0,

    /// @brief Booted via a Multiboot loader, expect g_Multiboot to be filled out.
    BM_Multiboot = 1,

    /// @brief Booted via (U)EFI system    
    BM_EFI       = 2,
};

BootMethod g_BootMethod;

/*************************************************************************/

void Arch::InitBootMemory(KernelArgs *ka)
{
    Debug::PrintF("ENTER: Arch::InitBootMemory\r\n");

    Debug::PrintF("Boot Method: %d\r\n", g_BootMethod);

    switch (g_BootMethod)
    {
    case BootMethod::BM_Multiboot:
        Multiboot::InitMultibootMemory(ka);
        break;

    case BootMethod::BM_Unknown:
    default:
        break;
    }
}

/*************************************************************************/
