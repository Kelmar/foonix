#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>

#include "PageFrameAllocator.h"

/*************************************************************************/

size_t NextPower2(size_t v)
{
    size_t i = 1;

    while (i < v)
        i <<= 1;

    return i;
}

namespace
{
    constexpr size_t SubLog2(size_t v, size_t l, size_t i)
    {
        return l >= v ? i : SubLog2(v, l << 1, i + 1);
    }
}

constexpr size_t Log2(size_t v) { return SubLog2(v, 1, 0); }

/*************************************************************************/

const PageBlock PageBlock::Nil(0, 0);

/*************************************************************************/

PageFrameAllocator::PageFrameAllocator(size_t maxSize)
    : m_maxSize(maxSize)
    , m_freePool()
{
    ASSERT(maxSize > PAGE_SIZE, "Invalid memory size.");

    // Add a node for all of memory
    size_t pow2 = NextPower2(m_maxSize);
    size_t order = GetOrder(pow2);

    BuddyNode *root = AllocNode();

    root->Start = 0;
    root->Size = pow2;

    m_buckets[order].Insert(root->Start, root);
}

PageFrameAllocator::~PageFrameAllocator(void)
{
}

/*************************************************************************/

PageFrameAllocator::BuddyNode *PageFrameAllocator::GetFreeNode(void)
{
    BuddyNode *node = m_freePool.Next;

    if (node == &m_freePool)
        return nullptr; // Out of frames

    m_freePool.Next = node->Next;
    node->Next = nullptr;

    return node;
}

/*************************************************************************/

void PageFrameAllocator::ReleaseNode(BuddyNode *node)
{
    if (!node)
        return;

    // Pedantic protection
    node->Start = 0;
    node->Size = 0;

    // Drop the frame back into the pool, we'll use it again later.
    node->Next = m_freePool.Next;
    m_freePool.Next = node;
}

/*************************************************************************/

PageFrameAllocator::BuddyNode *PageFrameAllocator::AllocNode(void)
{
    /*
     * In the kernel we should check the current page if there is space.
     * We need to make sure we always have space for nodes.
     */
    //printf("Allocating new node\r\n");
    return new BuddyNode();
}

/*************************************************************************/

PageFrameAllocator::BuddyNode *PageFrameAllocator::GetExactFrame(uintptr_t address, size_t order)
{
    BuddyNode *rval = m_buckets[order].Find(address);

    if (rval)
    {
        m_buckets[order].Remove(rval->Start);
        return rval;
    }

    if (order >= BucketSize - 1)
        return nullptr; // No more free memory!

    size_t size = PAGE_SIZE << order;

    int odd = size == 0 ? 0 : ((address / size) & 1);

    uintptr_t higherAddr = address - (odd ? size : 0);

    BuddyNode *split = GetExactFrame(higherAddr, order + 1);

    if (!split)
        return nullptr; // Address already allocated

    BuddyNode *remain = GetFreeNode();

    if (remain == nullptr)
        remain = AllocNode();

    remain->Start = split->Start + size;
    remain->Size = size;
    split->Size = size;
    remain->Next = nullptr;

    if (odd)
        std::swap(split, remain);

    m_buckets[order].Insert(remain->Start, remain);

    return split;
}

/*************************************************************************/

PageFrameAllocator::BuddyNode *PageFrameAllocator::GetAvailableNode(size_t order)
{
    BuddyNode *rval = m_buckets[order].Min();

    if (rval)
    {
        m_buckets[order].Remove(rval->Start);
        return rval;
    }

    if (order >= BucketSize - 1)
        return nullptr; // No more free memory!

    int size = PAGE_SIZE << order;

    // Recursively split
    BuddyNode *split = GetAvailableNode(order + 1);

    if (!split)
        return nullptr; // No more free memory!

    BuddyNode *remain = GetFreeNode();

    if (remain == nullptr)
        remain = AllocNode();

    remain->Start = split->Start + size;
    remain->Size = size;
    split->Size = size;
    remain->Next = nullptr;

    m_buckets[order].Insert(remain->Start, remain);

    return split;
}

/*************************************************************************/

PageBlock PageFrameAllocator::Aquire(uintptr_t address, size_t size)
{
    if (size == 0)
        return PageBlock::Nil;

    if (size >= m_maxSize)
        return PageBlock::Nil; // Cannot allocate the entirity of memory.

    if (size < PAGE_SIZE)
        size = PAGE_SIZE;

    uintptr_t alignedAddr = Align(address);
    size = SizeToBlock(size + (alignedAddr - address));

    size_t pow2 = NextPower2(size);
    int frameCnt = pow2 / PAGE_SIZE;

    /*
     * Currently we're just effectively allocating lots of small blocks
     * for the range we want, which isn't a very efficent way of doing
     * this, but it at least works for now.
     */

    // Shouldn't use a flat array like this.
    BuddyNode *frames[1024] = { nullptr };
    int index = 0;

    while (index < frameCnt)
    {
        frames[index] = GetExactFrame(alignedAddr, 0);

        if (frames[index] == nullptr)
        {
            // Could not allocate all needed frames!
            return PageBlock::Nil;
        }

        ++index;
        alignedAddr += PAGE_SIZE;
    }

    PageBlock rval(frames[0]->Start, pow2);

    for (int index = 0; index < frameCnt; ++index)
        ReleaseNode(frames[index]);

    return rval;
}

/*************************************************************************/

PageBlock PageFrameAllocator::Allocate(size_t requested)
{
    if (requested == 0)
        return PageBlock::Nil;

    if (requested >= m_maxSize)
        return PageBlock::Nil; // Cannot allocate the entirity of memory.

    if (requested < PAGE_SIZE)
        requested = PAGE_SIZE;

    size_t pow2 = NextPower2(requested);

    size_t order = GetOrder(pow2);

    BuddyNode *frame = GetAvailableNode(order);

    if (frame == nullptr)
        return PageBlock::Nil; // Out of memory!

    PageBlock rval(frame->Start, frame->Size);

    ReleaseNode(frame);

    return rval;
}

/*************************************************************************/

void PageFrameAllocator::Release(PageBlock &&block)
{
    if (!block)
        return;

    PageBlock b(std::move(block));

    BuddyNode *frame = GetFreeNode();

    if (!frame)
        frame = AllocNode();

    // Need to check that start and size inside the block are valid.

    frame->Start = b.Start;
    frame->Size = b.Size;

    size_t order = GetOrder(frame->Size);
    
    while (order < BucketSize - 1)
    {
        // Figure out if we're to the left or right of our buddy.
        int odd = (frame->Start / frame->Size) & 1;

        BuddyNode *buddy;

        if (odd)
        {
            // Our buddy comes before us
            buddy = m_buckets[order].Find(frame->Start - frame->Size);
        }
        else
        {
            // Our buddy comes after us
            buddy = m_buckets[order].Find(frame->End());
        }
        
        if (!buddy)
            break; // Buddy is allocated, exit loop.

        //printf("Merging frames\r\n");

        // Merge the frames together.
        m_buckets[order].Remove(buddy->Start);
        frame->Start = std::min(frame->Start, buddy->Start);
        frame->Size += buddy->Size;
        ++order;

        ReleaseNode(buddy);
    }

    //ASSERT(order < BucketSize, "Wrong bucket number!");

    m_buckets[order].Insert(frame->Start, frame);
}

/*************************************************************************/

bool PageFrameAllocator::CheckAllFree(void)
{
    size_t cnt = 0;

    for (size_t i = 0; i < BucketSize; ++i)
        cnt += m_buckets[i].Count();

    return cnt == 1;
}

/*************************************************************************/
