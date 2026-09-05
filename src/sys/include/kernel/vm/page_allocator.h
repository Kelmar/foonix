/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGE_ALLOCATOR_H__
#define __FOONIX_VM_PAGE_ALLOCATOR_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <type_traits>

#include <kernel/types.h>
#include <kernel/thread/spinlock.h>

/********************************************************************************************************************/

namespace paging
{
    /*
     * TODO: Split this class.
     *
     * This class is really at the moment a hybrid of a physical and a virtual page allocator.  Really these should
     * be two distinct things:
     *
     * 1) A physical allocator: Only one of these needs to ever be created, and would manage all of the system memory.
     *    It would not worry about what is or isn't in a page table or virtual memory map of whatever, it just would
     *    make sure available pages are handed out and marked, or unmarked when returned.
     *
     * 2) A virtual allocator: Would request pages from the physical allocator as needed, this would part would take
     *    care of making sure those pages get mapped to the places where they are needed.  E.g. stack, code, heap,
     *    or whatever else needs to be done.  There would be multiple allocators, one for each process structure.
     *
     */

    class PageAllocator
    {
    private:
        /*
         * We take a bit of the same approach as FreeBSD, and allocate a number of structures that track info about
         * each structure and how it's been split.  These structures are additionally placed into a free list bucket,
         * based on their size.
         */
        struct PageNode
        {
            PageNode *Prev;
            PageNode *Next;

            //PagePointer Pointer;
            uintptr_t Pointer;
        };

        // Think it's possible to do this lock free, but we'll play it safe for now.
        mutable thread::SpinLock m_allocLock;

        /// @brief Size of the m_pages array.
        PageNode *m_pages; // Array of page nodes, one per page on the system.

        /// @brief Also the size of the m_pages array.
        size_t m_totalPages;

        /// @brief Number of pages available.
        size_t m_pagesAvailable;

        // List of pages that have been linked to each other.
        PageNode *m_pageCache;
        size_t m_pageCacheCount; // TODO: Ideally this would be a semaphore

    private: // Unguarded methods
        void AddPageToCache(paddr_t addr);
        paddr_t GetCachedPage();

    private:
        void AllocateNodes();
        void InitBootPages();

    public:
        PageAllocator() noexcept;
        ~PageAllocator() { }

        size_t TotalPages() const
        {
            // No lock needed, this value is initialized on start and shouldn't change afterwards.
            return m_totalPages;
        }

        /// @brief Get the total number of pages available in the system.
        size_t GetFreePages() const;

        /// @brief Get the number of pages currently in the page cache.
        size_t GetCacheSize() const;

        /// @brief Allocate a single page and return its physical address.
        paddr_t AllocatePage();

        /// @brief Allocate a page and return it as a phyiscal pointer of the given type.
        template <typename T>
        inline T *AllocatePageAs()
        {
            return reinterpret_cast<T *>(AllocatePage());
        }

        /// @brief Return a page to the allocator for use by other things.
        void ReleasePage(paddr_t page);

        // Temporary
        void ReleasePage(void *ptr) { ReleasePage(reinterpret_cast<paddr_t>(ptr)); }
    };
}

/********************************************************************************************************************/

extern paging::PageAllocator page_allocator;

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGE_ALLOCATOR_H__ */

/********************************************************************************************************************/
