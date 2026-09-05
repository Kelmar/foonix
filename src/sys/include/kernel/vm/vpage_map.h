/********************************************************************************************************************/
/*
 * Virtual Page Allocator
 *
 * Manages page mappings for a process.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_VM_VPAGE_ALLOCATOR_H__
#define __FOONIX_VM_VPAGE_ALLOCATOR_H__

#include <expected>

#include <kernel/kernel.h>

#include <kernel/vm/page_table.h>

/********************************************************************************************************************/

namespace memory
{
    /**
     * @brief The type of memory requested.
     *
     * @remarks This informs the VPageAllocator where it should map the newly allocated pages of memory to.
     * In the future it might be worth while to allow the creation of different types of maps.
     */
    enum class MappingType
    {
        /// @brief Request to map new space into process heap space.
        Heap     = 1,

        /// @brief Request to map new space into a process's stack space.
        Stack    = 2,

        /// @brief Add additional memory to a process's code space.
        Text     = 3,

        /// @brief Request is for a specific physical page. (Typically for memory mapped hardware I/O)
        Physical = 4
    };

    class VPageMap;

    class VPageMapBuilder
    {
    private:
        paging::PageTable *m_sourceTable;

        bool m_useTable;

        vaddr_t m_stackStart;
        vaddr_t m_codeStart;
        vaddr_t m_heapStart;
        
        friend VPageMap;

        void FillDefaults();

        paging::PageTable *BuildPageTable();

        //VPageMap *BootBuild();

        //friend vmm::Init;

    public:
        constexpr VPageMapBuilder() noexcept;
        ~VPageMapBuilder() { }

        VPageMapBuilder &UseTable(paging::PageTable *table)
        {
            m_sourceTable = table;
            m_useTable = true;
            return *this;
        }

        VPageMapBuilder &CopyTable(paging::PageTable *table)
        {
            m_sourceTable = table;
            m_useTable = false;
            return *this;
        }

        VPageMapBuilder &CodeStart(vaddr_t address)
        {
            m_codeStart = address;
            return *this;
        }

        VPageMapBuilder &HeapStart(vaddr_t address)
        {
            m_heapStart = address;
            return *this;
        }
        
        memory::VPageMap *Build();
    };

    class VPageMap
    {
    private:
        paging::PageTable *m_sourceTable;

        vaddr_t m_nextStack;
        vaddr_t m_nextVirtual;

        VPageMap(VPageMapBuilder *builder, paging::PageTable *table) noexcept;

    public:
        ~VPageMap() { }

        /**
         * @brief Allocate pages of memory for the given type.
         *
         * @details This function will allocate one or more pages of physical
         * memory and map them to the the process's virtual memory space and return
         * the virtual address of the newly allocated virtual memory.
         *
         * Note that physical must be a valid address if the type specified is Physical.
         */
        std::expected<vaddr_t, kernel::ErrorCode>
        Allocate(MappingType type, size_t count = 1, paddr_t physical = 0);

        /// @brief Release one or more pages back to the physical page allocator.
        void Release(vaddr_t address, size_t count = 1);
    };
}

namespace kernel
{
    /// @brief Kernel main virtual memory map allocator, used by kalloc.
    extern memory::VPageMap *page_map;
}

/********************************************************************************************************************/

#endif /* __FOONIX_VM_VPAGE_ALLOCATOR_H__ */

/********************************************************************************************************************/
