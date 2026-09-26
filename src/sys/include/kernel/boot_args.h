/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARGS_H__
#define __FOONIX_KERNEL_ARGS_H__

/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel.h>
#include <kernel/utils/span.h>
#include <kernel/utils/nocopy.h>
#include <kernel/memory_range.h>

/********************************************************************************************************************/

namespace memory
{
    class BootPageAllocator;
}

/********************************************************************************************************************/

namespace boot
{
    /**
     * @brief Type of memory detected by bootloader.
     *
     * @remarks Note that currently these match the x86 BIOS values.
     */
    enum class MemoryType : uint32_t
    {
        /// @brief Invalid entry
        Invalid   = 0,

        /// @brief Memory is available for use
        Available = 1,

        /// @brief Memory has been reserved by the BIOS
        Reserved  = 2,

        /// @brief Memory is used for ACPI
        ACPI      = 3,

        /// @brief Memory is used for ACPI NVS
        ACPI_VMS  = 4,

        /// @brief Memory has been marked as bad
        BadMemory = 5
    };

    /************************************************************************************************************/

    struct MemoryMapping
    {
        uintptr_t Start;
        size_t Length;
        MemoryType Type;

        inline
        constexpr boot::MemoryMapping &operator =(const MemoryRange &rhs)
        {
            Start = rhs.Base;
            Length = rhs.Length;
            return *this;
        }

        inline
        constexpr MemoryRange ToRange() const
        {
            return MemoryRange(Start, Length);
        }
    };

    /************************************************************************************************************/

    /// @brief Describes details of the frame buffer that was setup by the boot loader.
    struct FrameBufferInfo
    {
        /// @brief Address of the first pixel in the frame buffer.
        paddr_t Address;

        /// @brief Width of the frame buffer in pixels.
        uint32_t Width;

        /// @brief Height of the frame buffer in pixels.
        uint32_t Height;

        /// @brief Pitch of the frame buffer in bytes.
        uint32_t Pitch;

        /// @brief Color depth of the frame buffer in bits per pixel.
        uint32_t Depth;
    };

    /************************************************************************************************************/
    /**
    * @brief Kernel argument data, this is allocated by the architecture in it's preboot phase and passed to the kernel
    * main() as an argument.  main() will then initialize the global KernelArgs object with these data.
    */
    struct ArgumentData
    {
        static constexpr size_t MaxMemoryEntries = 32;
        static constexpr size_t MaxCommandLine = 256;

        char CommandLine[MaxCommandLine];

        /// @brief Pointer to a BootPageAllocator object, this value might be null.
        memory::BootPageAllocator *BootPageAllocator;

        /// @brief The magic number from the boot loader.
        uint32_t BootMagicNumber;

        /**
         * @brief Size of the memory below 1MB in KBytes
         *
         * @remarks Effectively this is the reserved special memory area in kbytes.
         */ 
        size_t LowMemorySizeKByte;

        /**
         * @brief Size of the memory above 1MB in KBytes
         *
         * @remarks The effective normal memory available in kbytes.
         */
        size_t HighMemorySizeKByte;

        /**
        * @brief Physical memory location where the kernel starts.
        *
        * @remarks This is the unaligned start of the kernel.
        */
        paddr_t KernelStart;

        /**
        * @brief Last physical address the kernel code is occupying.
        *
        * @remarks This is the unaligned end of the kernel.
        */
        paddr_t KernelEnd;

        /// @brief Physical address of where the kernel heap starts.
        paddr_t HeapStart;

        /// @brief Physical address of the next available heep address is.
        paddr_t HeapNext;

        /// @brief Virtual address of the start of the kernel heap.
        vaddr_t VirtHeapStart;

        /// @brief Virtual address of the next available kernel heap.
        vaddr_t VirtHeapNext;

        /// @brief Number of valid entries in the MemoryMap table.
        size_t MemoryMapCount;

        /// @brief List of available memory.
        MemoryMapping MemoryMap[MaxMemoryEntries];

        FrameBufferInfo FrameBuffer;
    };

    /************************************************************************************************************/

    class Arguments : private util::nocopy, util::nomove
    {
    private:
        ArgumentData *m_data;

        /// @brief Set if allocating pages is currently safe.
        bool m_canAllocPages;

        /// @brief Set if calling kalloc() and friends is safe.
        bool m_canKalloc;

        /*
         * @brief Sort memory mappings so they appear in order.
         */
        void SortMappings();

        /// @brief Remove empty (zero length) mappings.
        void RemoveDeadMappings();

        /*
         * @brief Merge contiguous memory mappings into single maps.
         *
         * Attempt to crunch down memory map usage by merging contiguous memory map entries into larger single entries.
         */
        void MergeContiguousMappings();

        /*
         * @brief Slide MemoryMap entries down starting from start and continuing to the end.
         *
         * Reduces the length of the MemoryMap by exactly one.
         */ 
        void SlideEntries(int start);

    private:
        static Arguments s_instance;

        constexpr Arguments() noexcept : m_data(nullptr) { }

    public:
        static void Init(ArgumentData *data)
        {
            s_instance.m_data = data;
            s_instance.m_canAllocPages = false;
            s_instance.m_canKalloc = false;
        }

        /**
         * @brief Global kernel arguments structure.
         */
        static Arguments &Instance() { return s_instance; }

    public:
        /// @brief Physical location of the kernel.
        MemoryRange KernelRange() const
        {
            return MemoryRange::FromAddresses(m_data->KernelStart, m_data->KernelEnd);
        }

        /// @brief Physical location of the boot heap.
        MemoryRange HeapPhysical() const
        {
            return MemoryRange::FromAddresses(m_data->HeapStart, m_data->HeapNext); 
        }

        /// @brief Virtual location of the boot heap.
        MemoryRange HeapVirtual() const
        {
            return MemoryRange::FromAddresses(m_data->VirtHeapStart, m_data->VirtHeapNext);
        }

        bool CanAllocPages() const { return m_canAllocPages; }

        void CanAllocPages(bool value) { m_canAllocPages = value; }

        bool CanKalloc() const { return m_canKalloc; }

        void CanKalloc(bool value) { m_canKalloc = value; }

        /**
        * @brief Adds a potential range of free memory from a detected memory map.
        *
        * Does not attempt to resolve if memory is already used or not, simply adds it to
        * the initial bootup memory mapping provided by BIOS, Multiboot, hardware detection
        * or whatever.
        *
        * Note that this function may condense multiple mappings into one if they are
        * contiguous regions of memory.
        *
        * @returns True if the memory was successfully added to the map.  False if there
        * was not enough space in the map table to add the mapping.
        */
        bool AddMemoryMap(paddr_t base, size_t length, MemoryType type);

        /// @brief Gets a readonly span into the memory map entries.
        inline
        const util::span<MemoryMapping> MemoryMap() const
        {
            return util::span<MemoryMapping>(m_data->MemoryMap, m_data->MemoryMapCount);
        }

        /**
        * @brief Removes used memory from boot memory map.
        *
        * Runs over the detected free memory and removes any memory used by kernel and drivers
        * loaded by multiboot, EFI or other boot loader modules that weren't detected/removed
        * from the call to the AddMemoryMap() function.
        */
        void KnockoutUsedMemory();

        /// @brief Display the list of available memory ranges.
        void ShowAvailableMemory(void);
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARGS_H__ */

/********************************************************************************************************************/
