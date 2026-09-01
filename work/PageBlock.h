#pragma once

#include <stdint.h>
#include <stddef.h>

#include <utility>

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
