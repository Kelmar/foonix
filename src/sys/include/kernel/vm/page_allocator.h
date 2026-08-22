/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_VM_PAGE_ALLOCATOR_H__
#define __FOONIX_VM_PAGE_ALLOCATOR_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/thread/spinlock.h>

/********************************************************************************************************************/

namespace paging
{
    class PageAllocator
    {
    private:
        struct PageNode { PageNode *next; };

        // Think it's possible to do this lock free, but we'll play it safe for now.
        mutable thread::SpinLock m_allocLock;

        // List of pages that have been linked to each other.
        PageNode *m_pageCache;
        size_t m_pageCacheCount; // TODO: Ideally this would be a semaphore

    private: // Unguarded methods
        void AddPageToCache(paddr_t addr);
        paddr_t GetCachedPage();

    private:
        void InitBootPages();

    public:
        PageAllocator() noexcept;
        ~PageAllocator() { }

        /// @brief Get the total number of pages available in the system.
        size_t GetFreePages() const;

        /// @brief Get the number of pages currently in the page cache.
        size_t GetCacheSize() const;

        /**
         * @brief Allocate a single page and return its physical address.
         */
        paddr_t AllocatePage();

        /**
         * @brief Return a page to the allocator for use by other things.
         */
        void ReleasePage(paddr_t page);
    };
}

/********************************************************************************************************************/

extern paging::PageAllocator page_allocator;

/********************************************************************************************************************/

#endif /* __FOONIX_VM_PAGE_ALLOCATOR_H__ */

/********************************************************************************************************************/
