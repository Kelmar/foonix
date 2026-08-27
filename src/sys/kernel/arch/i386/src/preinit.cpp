/********************************************************************************************************************/

#include <kernel/arch.h>
#include <kernel/arch/dconsole.h>

#include "asm.h"
#include "cpu.h"
#include "idt.h"

#include "bootinfo.h"

#include "multiboot.h"
#include "multiboot2.h"

#include "paging.h"

/********************************************************************************************************************/

static BootInfo g_bootInfo;

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
void InitBootInfo(BootInfo *bootInfo)
{
    memset(bootInfo, 0, sizeof(BootInfo));

    // Figure out where we live in physical memory.

    // _kernel_phys_start is already set at physical address
    bootInfo->KernelStart = reinterpret_cast<paddr_t>(&_kernel_phys_start);

    // _kernel_end is set at virtual address.
    bootInfo->KernelEnd = reinterpret_cast<paddr_t>(VIRT_2_PHYS(&_kernel_end));

    // Take a stab at what we hope are some sane starting values if we don't know anything.
    bootInfo->LowMemorySize = 1024; // Assume 1MB low memory
    bootInfo->HighMemorySize = 1024 * 3; // Assume 3MB high memory

    // Free memory info from 0 to LowMemSize
    bootInfo->MemoryInfo[0].Start = 0;
    bootInfo->MemoryInfo[0].Length = bootInfo->LowMemorySize;
    bootInfo->MemoryInfo[0].Type = BiosMemoryType::Available;

    // Free memory info from LowMemSize to HighMemSize
    bootInfo->MemoryInfo[1].Start = bootInfo->LowMemorySize;
    bootInfo->MemoryInfo[1].Length = bootInfo->HighMemorySize;
    bootInfo->MemoryInfo[1].Type = BiosMemoryType::Available;
}

/********************************************************************************************************************/

static
void InitHeapInfo(BootInfo *bootInfo)
{
    if (bootInfo->HeapStart == 0)
    {
        // HeapStart wasn't set, try to guess at one.

        // First try for the page after the kernel's BSS
        paddr_t target = paging::AlignNext(bootInfo->KernelEnd);

        // AlignNext so we get an extra page for 4MB identity map as well.
        size_t minNeededBytes = paging::AlignNext(bootInfo->KernelEnd - bootInfo->KernelStart);

        // Find any free area after the kernel's BSS
        for (size_t i = 0; i < bootInfo->MemoryInfoCount; ++i)
        {
            if (bootInfo->MemoryInfo[i].Type != BiosMemoryType::Available)
                continue;

            paddr_t alignedStart = paging::AlignCeiling(bootInfo->MemoryInfo[i].Start);

            if (alignedStart < target)
                continue; // Skip memory before the end of the kernel BSS

            size_t adjust = reinterpret_cast<size_t>(alignedStart - bootInfo->MemoryInfo[i].Start);

            if (adjust > bootInfo->MemoryInfo[i].Length)
                continue; // Not even a full page!

            size_t length = bootInfo->MemoryInfo[i].Length - adjust;

            if (length < minNeededBytes)
                continue; // Not enough pages needed to map the kernel.

            target = alignedStart;
            break; // We found a suitable entry.
        }

        bootInfo->HeapStart = target;
    }

    if (bootInfo->HeapNext == 0)
        bootInfo->HeapNext = bootInfo->HeapStart;
}

/********************************************************************************************************************/
/**
 * @brief Pre-init function called from ASM to get basic page tables setup.
 *
 * @param magicNumber Boot loader detection magic number.
 * @param eax
 *
 * @details Loads the initial page directory and page table and enables paging on the CPU.
 *
 * We also parse the memory layout structures here. (E.g. Multiboot) as well as do any basic CPU detection that we might
 * need on start.
 */
extern "C"
void preinit(uint32_t magicNumber, uint32_t eax)
{
    DebugConsole::Init1();

    BootInfo *bootInfo = &g_bootInfo;
    InitBootInfo(bootInfo);
    
    // Now try to parse any information from the boot loader if we got it; they will overwrite anything that isn't correct.
    
    switch (magicNumber)
    {
    case MULTIBOOT_MAGIC:
        bootInfo->BootMagicNumber = magicNumber;
        Multiboot::ReadInfo(bootInfo, eax);
        break;

    case MB2_MAGIC:
        bootInfo->BootMagicNumber = magicNumber;
        MB2::ReadInfo(bootInfo, eax);
        break;
    }

    InitHeapInfo(bootInfo);

    //Debug::PrintF("Removing kernel usage from memory map.\r\n");

    // Remove any memory used by boot loader (e.g. Kernel code space)
    //ka->KnockoutUsedMemory();

    // Get paging setup.
    paging::Preinit(bootInfo);
}

/********************************************************************************************************************/

void init_idt(); // TODO: Put this decl in a header.

void arch::Init(KernelArgs *ka)
{
    BootInfo *bootInfo = &g_bootInfo;
    ka->SetCommandLine(bootInfo->CommandLine, BootInfo::CmdLineSize);
    ka->KernelCode = MemoryRange::FromAddresses(bootInfo->KernelStart, bootInfo->KernelEnd);
    ka->MemorySizeKByte = bootInfo->LowMemorySize + bootInfo->HighMemorySize;
    ka->HeapStart = bootInfo->HeapStart;
    ka->HeapNext = bootInfo->HeapNext;

    for (size_t i = 0; i < bootInfo->MemoryInfoCount; ++i)
    {
        if (bootInfo->MemoryInfo[i].Type != BiosMemoryType::Available)
            continue;

        ka->AddMemoryMap(bootInfo->MemoryInfo[i].Start, bootInfo->MemoryInfo[i].Length);
    }

    init_idt();
}

/********************************************************************************************************************/
