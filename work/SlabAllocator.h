#pragma once

#include <stddef.h>
#include <stdint.h>

#include "common.h"

struct SlabBucket
{
    SlabBucket* next;
    uintptr_t freeList;
    uintptr_t page;
};

template <typename T>
class SlabAllocator
{
public:
    typedef T Type;

private:
    const size_t ChunkSize     = MAX(sizeof(void*), sizeof(T));
    const size_t ChunksPerSlab = PAGE_SIZE / ChunkSize;

    union Chunk
    {
        Chunk* next;
        Type item;
    };

    SlabBucket* m_buckets;

    void AllocateBucket(void)
    {
        SlabBucket* s = new SlabBucket(); // Pull from slab slab allocator.
        Chunk* n;

        // Initialize the slab
        s->next = m_buckets;
        s->freeList = s->page;
        n = static_cast<Chunk*>(s->freeList);

        for (size_t i = 0; i < ChunksPerSlab; ++i)
            n->next = n + 1;

        m_buckets = s;
    }

    void ReleaseBucket(Slab* s)
    {
        delete s;
    }

public:
    // Remove copy construction.
    SlabAllocator(const SlabAllocator &) = delete;
    SlabAllocator &operator = (const SlabAllocator &) = delete;

    SlabAllocator()
        : m_buckets(nullptr)
    {
    }

    virtual ~SlabAllocator(void) noexcept
    {
    }

    Type *Allocate(void)
    {
        return nullptr;
    }

    void Release(Type *ptr)
    {

    }
};
