#pragma once

#include <stdint.h>
#include <stddef.h>

#include <utility>

#include "AVLTree.h"

#include "common.h"

#define PAGE_SIZE 4096
#define SYSTEM_BITS 32

size_t NextPower2(size_t v);
constexpr size_t Log2(size_t v);

class PageBlock
{
public:
    static const PageBlock Nil;

    uintptr_t Start;
    size_t    Size;

    PageBlock()
        : Start(0)
        , Size(0)
    {
    }

    PageBlock(uintptr_t start, size_t size)
        : Start(start)
        , Size(size)
    {
    }

    PageBlock(const PageBlock &rhs)
        : Start(rhs.Start)
        , Size(rhs.Size)
    {
    }

    PageBlock(PageBlock &&rhs) noexcept
        : Start(0)
        , Size(0)
    {
        std::swap(Start, rhs.Start);
        std::swap(Size, rhs.Size);
    }

    PageBlock &operator =(const PageBlock &rhs)
    {
        Start = rhs.Start;
        Size = rhs.Size;

        return *this;
    }

    operator bool(void) const noexcept
    {
        return Size != 0;
    }

    PageBlock &operator =(PageBlock &&rhs) noexcept
    {
        std::swap(Start, rhs.Start);
        std::swap(Size, rhs.Size);

        return *this;
    }
};

inline bool
operator ==(const PageBlock &lhs, const PageBlock &rhs)
{
    return lhs.Start == rhs.Start && lhs.Size == rhs.Size;
}

class PageFrameAllocator
{
private:
    static const uint32_t MinBucket = 12; // Log2(PAGE_SIZE);
    static const uint32_t BucketSize = SYSTEM_BITS - MinBucket;

    /****************************************************************/

    struct BuddyNode
    {
        uintptr_t Start;
        size_t Size;

        uintptr_t End(void) const { return Start + Size; }

        /// @brief Next buddy at this level.
        BuddyNode *Next;

        BuddyNode()
            : Start(0)
            , Size(0)
            , Next(this)
        {
        }
    };

    typedef AVLTree<uintptr_t, BuddyNode *> BucketType;

    /****************************************************************/

    size_t m_maxSize;

    //  Pool of free nodes (not free pages)
    BuddyNode m_freePool;

    // List of pages at a given bucket order
    BucketType m_buckets[BucketSize];

    /****************************************************************/

    uintptr_t Align(uintptr_t address) const
    {
        int remain = address % PAGE_SIZE;
        return address - remain;
    }

    size_t SizeToBlock(size_t size) const
    {
        if (size < PAGE_SIZE)
            size = PAGE_SIZE;
        else
            size = NextPower2(size);

        return size;
    }

    BuddyNode *GetFreeNode(void);
    void ReleaseNode(BuddyNode *node);

    BuddyNode *AllocNode(void);
    
    size_t GetOrder(size_t pow2)
    {
        return Log2(pow2) - MinBucket;
    }

    BuddyNode *GetAvailableNode(size_t order);
    BuddyNode *GetExactFrame(uintptr_t address, size_t order);

    /****************************************************************/

public:
    /* constructor */ PageFrameAllocator(size_t maxSize);
    virtual          ~PageFrameAllocator(void);

    PageBlock Aquire(uintptr_t address, size_t size);

    PageBlock Allocate(size_t requested);

    void Release(PageBlock &&pb);

    bool CheckAllFree(void);
};

