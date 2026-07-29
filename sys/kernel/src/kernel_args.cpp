/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>
#include <kernel/debug.h>

#include "cpu.h"

#include <algorithm>

/********************************************************************************************************************/
/**
 * @brief Sort memory mappings so that they appear in order.
 */
void KernelArgs::SortMappings()
{
    /*
     * Bubble sort entries to ensure order.
     *
     * Should be okay with only 16 entries.  At least I hope, and for lack of a much more complicated sort method
     * that I do not wish to write at the moment....
     *
     *              -- B.Simonds (July 26, 2026)
     */
    for (;;)
    {
        bool sorted = true;

        for (size_t i = 1; i < MemoryMapEntries; ++i)
        {
            MemoryRange &lastMapping = MemoryMap[i - 1];
            MemoryRange &mapping = MemoryMap[i];

            if (lastMapping.Base > mapping.Base)
            {
                std::swap(MemoryMap[i - 1], MemoryMap[i]);
                sorted = false;
            }
        }

        if (sorted)
            break;
    }
}

/********************************************************************************************************************/
/**
 * @brief Remove empty (zero length) mappings.
 */
void KernelArgs::RemoveDeadMappings()
{
    size_t index = 0;

    while (index < MemoryMapEntries)
    {
        if (MemoryMap[index].Length > 0)
        {
            ++index;
            continue;
        }

        // Slide values down one.
        SlideEntries(index);
    }
}

/********************************************************************************************************************/
/**
 * @brief Merge contiguous memory mappings into single maps.
 *
 * Attempt to crunch down memory map usage by merging contiguous memory map entries into larger single entries.
 */
void KernelArgs::MergeContiguousMappings()
{
    /*
     * This might be a bit pedantic, but we never know what has done the memory detection on our behalf.
     *
     * Here we attempt to crunch down the entries in the map to get things in a known state we can work with.
     */

    SortMappings();
    RemoveDeadMappings();
    
    // Second pass, merge contiguous blocks when found.
    for (size_t i = 1; i < MemoryMapEntries; ++i)
    {
        MemoryRange &lastMapping = MemoryMap[i - 1];
        MemoryRange &mapping = MemoryMap[i];

        const auto result = MemoryRange::Merge(lastMapping, mapping);

        if (result)
        {
            MemoryMap[i - 1] = result.value();

            // Slide remaining entries down one, and reprocess same index with new values.
            SlideEntries(i);

            --i; // Reprocess current index.
        }
    }
}

/********************************************************************************************************************/

void KernelArgs::SlideEntries(int start)
{
    for (size_t i = start + 1; i < MemoryMapEntries; ++i)
        MemoryMap[i - 1] = MemoryMap[i];

    --MemoryMapEntries;
}

/********************************************************************************************************************/

bool KernelArgs::AddMemoryMap(paddr_t addr, size_t length)
{
    if (length == 0)
    {
        // TODO: Warn on this?
        return true; // Do not attempt to add a zero length range.
    }

    MemoryRange newRange(addr, length);

    for (size_t i = 0; i < MemoryMapEntries; ++i)
    {
        auto result = MemoryRange::Merge(MemoryMap[i], newRange);

        if (result)
        {
            MemoryMap[i] = result.value();
            MergeContiguousMappings();
            return true;
        }
    }

    // If we get here, we need a new memory map.

    if (MemoryMapEntries + 1 >= MAX_MEMORY_ENTRIES)
        return false; // Out of space!

    MemoryMap[MemoryMapEntries++] = newRange;
    MergeContiguousMappings();
    return true;
}

/********************************************************************************************************************/
/**
 * @brief Removes used memory from boot memory map.
 *
 * Runs over the detected free memory and removes any memory used by kernel and drivers
 * loaded by multiboot, EFI or other boot loader modules that weren't detected/removed
 * from the call to the AddMemoryMap() function.
 */
void KernelArgs::KnockoutUsedMemory()
{
    paddr_t kernelEnd = KernelCode.End();

    SortMappings();

    for (size_t i = 0; i < MemoryMapEntries; ++i)
    {
        MemoryRange &mapping = MemoryMap[i];
        paddr_t mapEnd = mapping.End();

        if (mapEnd < KernelCode.Base)
            continue; // Haven't reached kernel yet.

        if (mapping.Base > kernelEnd)
            break; // We're past the end of the kernel; no need to check other sorted mappings.

        bool hasStartGap = KernelCode.Base > mapping.Base;
        bool hasEndGap = kernelEnd < mapEnd;

        if (!hasStartGap && !hasEndGap)
        {
            // The kernel has consumed this whole mapping.  Mark it for removal by RemoveDeadMappings()
            mapping.Length = 0;

            continue;
        }

        if (hasStartGap)
        {
            // Adjust current entry for start gap.
            mapping.Length = KernelCode.Base - mapping.Base;
        }
        
        if (hasEndGap)
        {
            if (hasStartGap)
            {
                // Kernel fits entirely within this mapping; which means it needs to be split.
                // First check to make sure we can fit the new mapping....

                if ((MemoryMapEntries + 1) >= MAX_MEMORY_ENTRIES)
                {
                    // I'm sure there's a better way to handle all of this, but for now... -- B.Simonds (July 26, 2026)
                    kpanic("Out of memory mappings for kernel boot, don't know what to do now...");
                }

                i = MemoryMapEntries++;
            }

            // Next available would be one past end of kernel.
            MemoryMap[i].Base = kernelEnd + 1;
            MemoryMap[i].Length = mapEnd - kernelEnd;

            if (hasStartGap)
                break; // No other mappings affected;
        }
    }

    RemoveDeadMappings();
}

/********************************************************************************************************************/

void KernelArgs::ShowAvailableMemory(void)
{
    Debug::PrintF("Free Memory\r\n");

    for (uint32_t i = 0; i < MemoryMapEntries; ++i)
    {
        Debug::PrintF("    %p %08X\r\n", MemoryMap[i].Base, MemoryMap[i].Length);
    }
}

/********************************************************************************************************************/
