#pragma once

#include "stdint.h"

#define ASSERT(X__, T__) do { if (!(X__)) { abort(); } } while(false)

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

inline constexpr
size_t MIN(size_t A, size_t B)
{
    return A < B ? A : B;
}

inline constexpr
size_t MAX(size_t A, size_t B)
{
    return A > B ? A : B;
}

