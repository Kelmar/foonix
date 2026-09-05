/********************************************************************************************************************/

#include <kernel/arch.h>
#include <kernel/arch/dconsole.h>
#include <kernel/debug.h>

#include <kernel/boot_args.h>
#include <kernel/kalloc.h>

#include <kassert.h>

#include "x86priv.h"

#include "asm.h"
#include "cpu.h"
#include "idt.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "paging.h"

/********************************************************************************************************************/

using namespace boot;

static ArgumentData g_kernArgData;

/********************************************************************************************************************/
/*
 * These are defined in the linker script.
 */

/// @brief Physical memory location of the start of the kernel.
extern "C" uintptr_t _kernel_phys_start;

/// @brief Physical memory location of the end of the kernel.
extern "C" uintptr_t _kernel_end;

/********************************************************************************************************************/

static
void InitBootInfo(ArgumentData *argData)
{
    memset(argData, 0, sizeof(ArgumentData));

    // Figure out where we live in physical memory.

    // _kernel_phys_start is already set at physical address
    argData->KernelStart = reinterpret_cast<paddr_t>(x86::Virt2Phys(&_kernel_phys_start));

    // _kernel_end is set at virtual address.
    argData->KernelEnd = reinterpret_cast<paddr_t>(x86::Virt2Phys(&_kernel_end));

    // Take a stab at what we hope are some sane starting values if we don't know anything.
    argData->LowMemorySizeKByte = 1024; // Assume 1MB low memory
    argData->HighMemorySizeKByte = 1024 * 3; // Assume 3MB high memory

    // Free memory info from 0 to LowMemSize
    argData->MemoryMap[0].Start = 0;
    argData->MemoryMap[0].Length = argData->LowMemorySizeKByte;
    argData->MemoryMap[0].Type = MemoryType::Available;

    // Free memory info from LowMemSize to HighMemSize
    argData->MemoryMap[1].Start = argData->LowMemorySizeKByte;
    argData->MemoryMap[1].Length = argData->HighMemorySizeKByte;
    argData->MemoryMap[1].Type = MemoryType::Available;

    argData->MemoryMapCount = 2;
}

/********************************************************************************************************************/
/**
 * @brief Try and guess at missing heap information.
 *
 * @details This function will attempt to make some educated guesses about where to place the heap, if the bootloader
 * parsing does not set this information.
 *
 * Specifically it will place the heap just after the kernel in both physical and virtual memory.
 */
static
void InitHeapInfo(ArgumentData *argData)
{
    if (argData->HeapStart == 0)
    {
        // HeapStart wasn't set, try to guess at one.

        // First try for the page after the kernel's BSS
        paddr_t target = paging::AlignNext(argData->KernelEnd);

        // AlignNext so we get an extra page for 4MB identity map as well.
        size_t minNeededBytes = paging::AlignNext(argData->KernelEnd - argData->KernelStart);

        // Find any free area after the kernel's BSS
        for (size_t i = 0; i < ArgumentData::MaxMemoryEntries; ++i)
        {
            if (argData->MemoryMap[i].Type != MemoryType::Available)
                continue;

            paddr_t alignedStart = paging::AlignCeiling(argData->MemoryMap[i].Start);

            if (alignedStart < target)
                continue; // Skip memory before the end of the kernel BSS

            size_t adjust = reinterpret_cast<size_t>(alignedStart - argData->MemoryMap[i].Start);

            if (adjust > argData->MemoryMap[i].Length)
                continue; // Not even a full page!

            size_t length = argData->MemoryMap[i].Length - adjust;

            if (length < minNeededBytes)
                continue; // Not enough pages needed to map the kernel.

            target = alignedStart;
            break; // We found a suitable entry.
        }

        argData->HeapStart = target;
        argData->HeapNext = argData->HeapStart;

        argData->VirtHeapStart = reinterpret_cast<vaddr_t>(x86::Phys2Virt(argData->HeapStart));
        argData->VirtHeapNext = argData->VirtHeapStart;
    }
    else
    {
        if (argData->HeapNext == 0)
            argData->HeapNext = argData->HeapStart;

        if (argData->VirtHeapStart == 0)
            argData->VirtHeapNext = reinterpret_cast<vaddr_t>(x86::Phys2Virt(argData->HeapNext));

        if (argData->VirtHeapNext == 0)
            argData->VirtHeapNext = argData->VirtHeapStart;
    }
}

/********************************************************************************************************************/
/**
 * @brief Pre-init function called from ASM to get basic page tables setup.
 *
 * @param magicNumber Boot loader detection magic number.
 * @param eax
 *
 * @details Called before main()
 *
 * Loads the initial page directory and page table and enables paging on the CPU.
 *
 * We also parse the memory layout structures here. (E.g. Multiboot) as well as do any basic CPU detection that we might
 * need on start.
 */
extern "C"
ArgumentData *preinit(uint32_t magicNumber, uint32_t eax)
{
    ArgumentData *data = x86::Virt2Phys(&g_kernArgData);
    InitBootInfo(data);

    /*
     * Leaving this here for now, but later might move past the bootloader info parse, so we can configure the serial port.
     *
     * This does mean that we won't have the luxury of using Debug::PrintF() inside of bootloader parsing.
     */
    DebugConsole::Init1();

    // Now try to parse any information from the boot loader if we got it; they will overwrite anything that isn't correct.
    
    switch (magicNumber)
    {
    case MULTIBOOT_MAGIC:
        data->BootMagicNumber = magicNumber;
        Multiboot::ReadInfo(data, eax);
        break;

    case MB2_MAGIC:
        data->BootMagicNumber = magicNumber;
        MB2::ReadInfo(data, eax);
        break;
    }

    InitHeapInfo(data);

    // Get paging setup.
    paging::Preinit(data);

    return x86::Phys2Virt(data); // Return pointer for call to kmain() from assembly.
}

/********************************************************************************************************************/

// TODO: Put these in proper headers.
void init_idt();
void init_pics();
void init_timer();

// Called from main()
void arch::Init()
{
    init_idt();
    init_pics();

    init_timer();

    cpu::start_interrupts();
}

/********************************************************************************************************************/
