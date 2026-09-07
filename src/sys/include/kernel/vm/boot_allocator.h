/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_BOOT_ALLOCATOR_H__
#define __FOONIX_VM_BOOT_ALLOCATOR_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/types.h>

#include <kernel/vm/page_allocator.h>

/********************************************************************************************************************/

namespace memory
{
    /**
     * @brief A simple boot up bump allocator.
     *
     * @details This is a boot up allocator that just implements a bump allocator, with the ability to handle 
     * a basic free list, using the pages themselves as nodes.
     *
     * An important note about this allocator is that it assumes that the pages are all identity mapped and available
     * in the kernel's page table to keep track of the freed pages.  This may not always be the case, but should be
     * a reasonably safe assumption for the kernel startup.
     *
     * Also note that this allocator does a very slow O(n) allocation with the requested number of pages.
     *
     * As such this allocator is not suitable for a fully running system; it's intention is to provide a dynamic
     * set of pages so we can bootstrap a fully functional buddy allocator with a variable sized list of page
     * tracking information.
     */ 
    class BootPageAllocator : PageAllocator
    {
    private:
        struct PageNode { PageNode *Next; };

        boot::ArgumentData *m_data;
        size_t m_totalPages;
        PageNode *m_free;
        size_t m_freeCount;

        void GetPagesFromMappings();

    public:
        BootPageAllocator(boot::ArgumentData *data) noexcept;

        virtual ~BootPageAllocator() { }

        size_t TotalPages() const override { return m_totalPages; }
        size_t GetFreePages() const override;

        size_t AllocatePages(size_t count, PagePointer pointers[]) override;
        void ReleasePages(util::span<PagePointer> ptrs) override;
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_VM_BOOT_ALLOCATOR_H__ */

/********************************************************************************************************************/

