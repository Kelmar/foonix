#pragma once

#include <stddef.h>
#include <stdint.h>

#include "common.h"

template <typename T>
class SlabAllocator
{
public:
    typedef T Type;

private:
    struct Cache
    {
        Cache *Next;
        char *Block;

        public Cache()
            : Next(nullptr)
            , Block(nullptr)
        {
        }
    };

    Cache *m_cacheLines;

public:
    // Remove copy construction.
    SlabAllocator(const SlabAllocator &) = delete;
    SlabAllocator &operator = (const SlabAllocator &) = delete;

    virtual ~SlabAllocator(void) noexcept
    {
        Cache *i, *n;

        for (i = m_cacheLines; (n = i ? i->Next : nullptr, i); i = n)
            delete i;
    }

    Type *Allocate(void)
    {
        return nullptr;
    }

    void Release(Type *ptr)
    {

    }
};
