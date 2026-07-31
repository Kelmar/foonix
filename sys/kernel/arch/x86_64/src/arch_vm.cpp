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
//#include "multiboot.h"
//#include "arch_vm.h"

/********************************************************************************************************************/

// The assembly code will initialize these values for us.
uint32_t g_BootMagic; /* Value from EAX register */
uint32_t g_Multiboot; /* Value from EBX register */

/********************************************************************************************************************/

void arch::InitBootMemory(KernelArgs *ka)
{
    (void)ka;
}

/********************************************************************************************************************/
