/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/boot_args.h>
#include <kernel/debug.h>

#include <algorithm>
#include <string.h>

#include "paging.h"

using namespace boot;

/********************************************************************************************************************/

boot::Arguments boot::Arguments::s_instance;

/********************************************************************************************************************/
/**
 * @brief Sort memory mappings so that they appear in order.
 */
void Arguments::SortMappings()
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

        for (size_t i = 1; i < m_data->MemoryMapCount; ++i)
        {
            MemoryMapping &lastMapping = m_data->MemoryMap[i - 1];
            MemoryMapping &mapping = m_data->MemoryMap[i];

            if (lastMapping.Start > mapping.Start)
            {
                std::swap(m_data->MemoryMap[i - 1], m_data->MemoryMap[i]);
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
void Arguments::RemoveDeadMappings()
{
    size_t index = 0;

    while (index < m_data->MemoryMapCount)
    {
        if (m_data->MemoryMap[index].Length > 0)
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
void Arguments::MergeContiguousMappings()
{
    /*
     * This might be a bit pedantic, but we never know what has done the memory detection on our behalf.
     *
     * Here we attempt to crunch down the entries in the map to get things in a known state we can work with.
     */

    SortMappings();
    RemoveDeadMappings();
    
    // Second pass, merge contiguous blocks when found.
    for (size_t i = 1; i < m_data->MemoryMapCount; ++i)
    {
        MemoryRange lastMapping = m_data->MemoryMap[i - 1].ToRange();
        MemoryRange mapping = m_data->MemoryMap[i].ToRange();

        const auto result = MemoryRange::Merge(lastMapping, mapping);

        if (result)
        {
            m_data->MemoryMap[i - 1] = result.value();

            // Slide remaining entries down one, and reprocess same index with new values.
            SlideEntries(i);

            --i; // Reprocess current index.
        }
    }
}

/********************************************************************************************************************/

void Arguments::SlideEntries(int start)
{
    for (size_t i = start + 1; i < m_data->MemoryMapCount; ++i)
        m_data->MemoryMap[i - 1] = m_data->MemoryMap[i];

    --m_data->MemoryMapCount;
}

/********************************************************************************************************************/

bool Arguments::AddMemoryMap(paddr_t addr, size_t length, MemoryType type)
{
    if (length == 0)
    {
        // TODO: Warn on this?
        return true; // Do not attempt to add a zero length range.
    }

    if (m_data->MemoryMapCount + 1 >= ArgumentData::MaxMemoryEntries)
    {
        Debug::PrintF("WARN: Attempt to add more entries than available in boot up memory map of %u\r\n", ArgumentData::MaxMemoryEntries);
        return false; // Out of space!
    }

    int idx = m_data->MemoryMapCount++;

    m_data->MemoryMap[idx].Start = addr;
    m_data->MemoryMap[idx].Length = length;
    m_data->MemoryMap[idx].Type = type;

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
void Arguments::KnockoutUsedMemory()
{
    // Get page aligned start/end
    MemoryRange kernelRange = KernelRange();

    paddr_t kernelStartAligned = kernelRange.BaseAligned();
    paddr_t kernelEndAligned = kernelRange.EndAligned();

    SortMappings();

    for (size_t i = 0; i < m_data->MemoryMapCount; ++i)
    {
        MemoryMapping &mapping = m_data->MemoryMap[i];
        MemoryRange mapRange { mapping.Start, mapping.Length };

        paddr_t mapEnd = mapRange.End();

        if (mapEnd < kernelStartAligned)
            continue; // Haven't reached kernel yet.

        if (mapping.Start > kernelEndAligned)
            break; // We're past the end of the kernel; no need to check other sorted mappings.

        bool hasStartGap = kernelStartAligned > mapping.Start;
        bool hasEndGap = kernelEndAligned < mapEnd;

        if (!hasStartGap && !hasEndGap)
        {
            // The kernel has consumed this whole mapping.  Mark it for removal by RemoveDeadMappings()
            mapping.Length = 0;

            continue;
        }

        if (hasStartGap)
        {
            // Adjust current entry for start gap.
            mapping.Length = kernelStartAligned - mapping.Start;
        }
        
        if (hasEndGap)
        {
            if (hasStartGap)
            {
                // Kernel fits entirely within this mapping; which means it needs to be split.
                // First check to make sure we can fit the new mapping....

                if ((m_data->MemoryMapCount + 1) >= ArgumentData::MaxMemoryEntries)
                {
                    // I'm sure there's a better way to handle all of this, but for now... -- B.Simonds (July 26, 2026)
                    kpanic("Out of memory mappings for kernel boot, don't know what to do now...");
                }

                i = m_data->MemoryMapCount++;
            }

            // Next available would be one past end of kernel.
            m_data->MemoryMap[i].Start = kernelEndAligned + 1;
            m_data->MemoryMap[i].Length = mapEnd - kernelEndAligned;

            if (hasStartGap)
                break; // No other mappings affected;
        }
    }

    RemoveDeadMappings();
}

/********************************************************************************************************************/

void Arguments::ShowAvailableMemory(void)
{
    Debug::PrintF("Free Memory\r\n");
    Debug::PrintF("    Start      Length\r\n");

    for (uint32_t i = 0; i < m_data->MemoryMapCount; ++i)
    {
        Debug::PrintF("    %p %08X\r\n", m_data->MemoryMap[i].Start, m_data->MemoryMap[i].Length);
    }
}

/********************************************************************************************************************/
