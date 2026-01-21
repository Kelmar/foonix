#include <utility>

#include <kernel/allocator.h>

/*************************************************************************/

constexpr
PageBlock::PageBlock(logical_addr_t start, size_t count)
    : m_start(start)
    , m_count(count)
{  
}

constexpr
PageBlock::PageBlock(const PageBlock &rhs)
    : m_start(rhs.m_start)
    , m_count(rhs.m_count)
{
}

PageBlock::PageBlock(PageBlock &&rhs)
    : m_start(0)
    , m_count(0)
{
    operator =(std::forward(rhs));
}

PageBlock::~PageBlock()
{
}

/*************************************************************************/

PageBlock &PageBlock::operator =(PageBlock &&rhs)
{
    std::swap(rhs.m_start, m_start);
    std::swap(rhs.m_count, m_count);

    return *this;
}

PageBlock &PageBlock::operator =(const PageBlock &rhs)
{
    m_start = rhs.m_start;
    m_count = rhs.m_count;

    return *this;
}

/*************************************************************************/

