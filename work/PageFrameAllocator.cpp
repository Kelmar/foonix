#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>

#include "PageFrameAllocator.h"

/*************************************************************************/

const PageBlock PageBlock::Nil(0, 0);

/*************************************************************************/

PageFrameAllocator::PageFrameAllocator(size_t maxSize)
    : m_maxSize(maxSize)
    , m_freePool()
{
    ASSERT(maxSize > PAGE_SIZE, "Invalid memory size.");

    // Add a node for all of memory
    size_t pow2 = ToPow2(m_maxSize);
    size_t order = GetOrder(pow2);
    uintptr_t index = 0;

    BuddyNode *root = new BuddyNode();

    if (pow2 != m_maxSize)
        pow2 >>= 1;

    while (pow2 >= PAGE_SIZE)
    {
        if (pow2 < maxSize)
        {
            root->Start = index;
            root->Size = pow2;

            maxSize -= pow2;
            index += pow2;

            m_buckets[order].Insert(root->Start, root);
        }

        pow2 >>= 1;
        --order;
    }
}

PageFrameAllocator::~PageFrameAllocator(void)
{
}

/*************************************************************************/

PageFrameAllocator::BuddyNode *PageFrameAllocator::GetFreeNode(void)
{
    BuddyNode *node = m_freePool.Next;

    if (node == &m_freePool)
    {
        //printf("Allocating new node\r\n");
        return new BuddyNode();
    }

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

    remain->Start = split->Start + size;
    remain->Size = size;
    split->Size = size;
    remain->Next = nullptr;

    m_buckets[order].Insert(remain->Start, remain);

    return split;
}

/*************************************************************************/

PageBlock PageFrameAllocator::Acquire(uintptr_t address, size_t size)
{
    if (size == 0)
        return PageBlock::Nil;

    if (size >= m_maxSize)
        return PageBlock::Nil; // Cannot allocate the entirety of memory.

    if (size < PAGE_SIZE)
        size = PAGE_SIZE;

    uintptr_t alignedAddr = Align(address);
    size += alignedAddr - address;

    size_t pow2 = ToPow2(size);
    int frameCnt = pow2 / PAGE_SIZE;
    int order = GetOrder(size);

    BuddyNode *frame = GetExactFrame(alignedAddr, order);

    if (!frame)
    {
        // Could not allocate all needed frames!
        return PageBlock::Nil;
    }

    PageBlock rval(frame->Start, frame->Size);

    for (int index = 0; index < frameCnt; ++index)
        ReleaseNode(frame);

    return rval;
}

/*************************************************************************/

PageBlock PageFrameAllocator::Allocate(size_t requested)
{
    if (requested == 0)
        return PageBlock::Nil;

    if (requested >= m_maxSize)
        return PageBlock::Nil; // Cannot allocate the entirety of memory.

    if (requested < PAGE_SIZE)
        requested = PAGE_SIZE;

    size_t pow2 = ToPow2(requested);

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
