/********************************************************************************************************************/
/*
 * Virtual Page Allocator
 */
/********************************************************************************************************************/

#include <expected>

#include <kernel/kernel.h>

#include <kernel/vm/page_table.h>
#include <kernel/vm/page_allocator.h>
#include <kernel/vm/vpage_map.h>

/********************************************************************************************************************/

using PageTable = paging::PageTable;
using ErrorCode = kernel::ErrorCode;

namespace memory;

/********************************************************************************************************************/

VPageMapBuilder::VPageMapBuilder() noexcept
    : m_sourceTable(nullptr)
    , m_stackStart(0)
    , m_heapStart(0)
    , m_codeStart(0)
{
}

/********************************************************************************************************************/

void VPageMapBuilder::FillDefaults()
{
    if (m_codeStart == 0)
    {
        m_codeStart = 0x0010'0000; // TODO: Fix this.
    }

    if (m_stackStart == 0)
    {
        vaddr_t a = kernel::page_map->GetMapStart(MemoryType::Text);
        m_stackStart = paging::AlignPrev(a);
    }

    if (m_heapStart == 0)
    {
        m_heapStart = m_codeStart + 0x1000'0000; // TODO: Fix this.
    }
}

/********************************************************************************************************************/

PageTable *VPageMapBuilder::BuildPageTable()
{
    if (m_useTable && m_sourceTable)
        return m_sourceTable;

    PageTable *rval = new PageTable();

    if (rval == nullptr)
        kpanic("Out of memory attempting to get new PageTable");

    return rval;
}

/********************************************************************************************************************/

VPageMap *VPageMapBuilder::BootBuild()
{
    //FillDefaults();

    //PageTable *table = BuildPageTable();
    //void *ptr = page_allocator.AllocatePageAs<void *>();
    //return new (ptr) VPageMap(*this, table);

    return nullptr;
}

/********************************************************************************************************************/

VPageMap *VPageMapBuilder::Build()
{
    FillDefaults();

    PageTable *table = BuildPageTable();
    return new VPageMap(*this, table);
}

/********************************************************************************************************************/
/********************************************************************************************************************/

VPageMap::VPageMap(const VPageMapBuilder &builder, PageTable *table) noexcept
    : m_pageTable(table)
    , m_nextStack(0)
    , m_nextHeap(0)
{
    m_nextStack = builder.m_stackStart;
    m_nextHeap = builder.m_heapStart;
}

VPageMap::~VPageMap()
{
}

/********************************************************************************************************************/

std::expected<vaddr_t, ErrorCode>
VPageMap::Allocate(MappingType type, size_t count /* = 1 */, paddr_t physical /* = 0 */)
{
    if (count == 0)
        return ErrorCode::InvalidFormat;

    if (type == MappingType::Physical))
    {
        if (physical == 0)
            return ErrorCode::InvalidAddress;

        return ErrorCode::Unknown; // Not yet supported.
    }

    paddr_t paddr = page_allocator.AllocatePage();

    if (!paddr)
    {
        // Honestly we might want to just panic here.
        return ErrorCode::OutOfMemory;
    }

    ErrorCode err = m_pageTable->MapPage(paddr, m_nextVirtual);

    if (err == ErrorCode::NoError)
        m_nextHeap += cpu::PageSize; // Basic bump allocation for now....
    else
        page_allocator.ReleasePage(paddr);

    return err;
}

/********************************************************************************************************************/

void VPageMap::Release(vaddr_t address, size_t count /* = 1 */)
{
    if (!address)
        return; // Ignore requests to unmap the NULL page.

    if (address != pageing::AlignFloor(address))
        kpanic("Request to VPageMap::Release() with unaligned page.");

    vaddr_t vaddr = address;

    /*
     * TODO: Release pages in blocks.  Do note that we have to check things
     * carefully here when doing so.  Virtual addresses might be consecutive, 
     * but physical pages might not be.
     */
    
    for (size_t i = 0; i < count; ++i, vaddr += cpu::PageSize)
    {
        paddr_t paddr = m_pageTable->GetPhysicalPageFor(vaddr);

        if (!paddr)
            continue; // Page isn't mapped.

        ErrorCode err = m_pageTable->UnmapPage(vaddr);

        if (err != ErrorCode::NoError)
        {
            Debug::PrintF("Error %d attempting to unmap page %p that was just mapped.\r\n", err, paddr);
            kpanic("Error attempting to unmap a page that was just mapped!\r\n");
        }

        // TODO: Mark our own map as this being released.

        kernel::page_allocator.ReleasePage(paddr);
    }
}

/********************************************************************************************************************/

