/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_ALLOCATOR_H__
#define __FOONIX_KERNEL_ALLOCATOR_H__

/*************************************************************************/

#include <stddef.h>

#include <utility>

#include <sys/assert.h>

#include <kernel/bitmap.h>
#include <kernel/kernel.h>

/*************************************************************************/

const int PAGE_SIZE = 4096; // TODO: Move this to platform.


class PageBlock
{
private:
    logical_addr_t m_start;
    size_t m_count;

public:
    /* constructor */ PageBlock(logical_addr_t start, size_t count)
        : m_start(start)
        , m_count(count)
    {
    }

    /* constructor */ PageBlock(const PageBlock &rhs)
        : m_start(rhs.m_start)
        , m_count(rhs.m_count)
    {
    }

    /* constructor */ PageBlock(PageBlock &&rhs)
        : m_start(0)
        , m_count(0)
    {
        std::swap(rhs.m_start, m_start);
        std::swap(rhs.m_count, m_count);
    }

    virtual ~PageBlock(void) { }

    logical_addr_t Address(void) const { return m_start; }
    size_t Count(void) const { return m_count; }

    PageBlock &operator =(PageBlock &&rhs)
    {
        std::swap(rhs.m_start, m_start);
        std::swap(rhs.m_count, m_count);

        return *this;
    }

    PageBlock &operator =(const PageBlock &rhs)
    {
        m_start = rhs.m_start;
        m_count = rhs.m_count;

        return *this;
    }

    operator bool(void) const { return m_start != 0 && m_count != 0; }

    operator void *(void) const { return reinterpret_cast<void *>(m_start); }
};

extern PageBlock NullBlock;

bool operator ==(const PageBlock &lhs, const PageBlock &rhs)
{
    return lhs.Address() == rhs.Address() && lhs.Count() == rhs.Count();
}

/*************************************************************************/

class PageAllocator
{
protected:
    /* constructor */ PageAllocator(void);

public:
    virtual PageBlock NewBlock(size_t count) = 0;
    virtual void FreeBlock(PageBlock &block) = 0;
};

/*************************************************************************/

template <typename T>
class Allocator
{
public:
    typedef T Type;
    static const size_t ObjectSize = sizeof(T);

    virtual T *Allocate(void) = 0;
    virtual void Release(void) = 0;
};

/*************************************************************************/

/**
 * @brief A simple slab allocator
 * 
 * This allocator only handles objects that are less than PAGE_SIZE
 */
template <typename T>
class SlabAllocator : public Allocator<T>
{
private:
    static const size_t ObjectsPerPage = PAGE_SIZE / sizeof(T);

    struct Cache
    {
        Cache *m_prev;
        Cache *m_next;

        Bitmap<ObjectsPerPage> m_allocated;

        /* constructor */ Cache()
            : m_prev(nullptr)
            , m_next(nullptr)
            , m_allocated()
        {
        }
    };

    Cache m_first;
    PageAllocator *m_pageAllocator;

public:
    /* constructor */ SlabAllocator(PageAllocator *pageAllocator)
        : m_first()
        , m_pageAllocator(pageAllocator)
    {
        ASSERT(m_pageAllocator, "Invalid page allocator passed to SlabAllocator");

        m_first.m_pageBlock = m_pageAllocator->NewBlock(1);
    }

    virtual ~SlabAllocator(void)
    {
    }
};

/*************************************************************************/

#endif /* __FOONIX_KERNEL_ALLOCATOR_H__ */

/*************************************************************************/
