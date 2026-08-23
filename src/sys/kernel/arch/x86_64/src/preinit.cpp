/********************************************************************************************************************/

#include <kernel/arch.h>
#include <kernel/arch/dconsole.h>

#include "asm.h"
#include "cpu.h"

#include "bootinfo.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "paging.h"

/********************************************************************************************************************/

extern "C"
void preinit(/* uint32_t magicNumber, uint32_t eax */)
{

}

void arch::Init(KernelArgs *)
{
}

/********************************************************************************************************************/
