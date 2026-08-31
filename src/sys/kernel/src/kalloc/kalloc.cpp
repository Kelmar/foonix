/********************************************************************************************************************/
/********************************************************************************************************************/

#include <type_traits>
#include <concepts>

#include <kassert.h>

#include <kernel/debug.h>
#include <kernel/kernel_args.h>

#include <kernel/utils/span.h>
#include <kernel/utils/list.h>
#include <kernel/utils/stack.h>

#include <kernel/thread/spinlock.h>
#include <kernel/thread/lockguard.h>

#include <kernel/vm/page_allocator.h>

#include <kernel/kalloc.h>

//#include "kalloc_config.h"
#define LG_BUCKET_SIZE 32
#define EMPTY_MAX 4

#include <kernel/kalloc/hash.h>
#include <kernel/kalloc/small_slab.h>
#include <kernel/kalloc/small_bucket.h>

/********************************************************************************************************************/

size_t kallocAllocatedPages = 0;

/********************************************************************************************************************/

struct LargePageMeta
{
    /// @brief Pointer to the previous metadata block.
    LargePageMeta *Prev;

    /// @brief Pointer to next block metadata structure.
    LargePageMeta *Next;

    /// @brief Pointer to the first page of this block
    void *PagePtr;

    /// @brief Number of pages allocated for this block
    size_t PageCount;

    constexpr LargePageMeta() noexcept
        : Next(nullptr)
        , PagePtr(nullptr)
        , PageCount(0)
    { }
};

/********************************************************************************************************************/

/// @brief Small item allocation buckets, hashed by size.
static
SmallItemBucket *smBuckets; //[SmallItemHash::MAX_BUCKET_INDEX];

/// @brief Global lock for lgBuckets (TODO: Replace this later.)
thread::SpinLock lgBucketMutex;

/// @brief Stack of allocated large meta objects, hashed by pointer.
static
List<LargePageMeta> *lgBuckets[LG_BUCKET_SIZE];

/// @brief Stack of free large meta objects.
static
Stack<LargePageMeta> freeLargeMetas;

/********************************************************************************************************************/

static inline
uint8_t hash_ptr(void *ptr)
{
    constexpr uint8_t HASH_MASK = LG_BUCKET_SIZE - 1;

    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);

    return static_cast<uint8_t>((p >> cpu::PageShift) & HASH_MASK);
}

static
void *kalloc_small(size_t sz);

/********************************************************************************************************************/

void memory::init_kalloc()
{
    Debug::PrintF("ENTER: memory::init_kalloc()\r\n");

    if (!kernel::arguments.CanAllocPages)
        kpanic("memory::init_kalloc(): Called before ability to allocate memory pages!\r\n");

    kallocAllocatedPages = 0;

    // TODO: Fix this later when we can allocate more than one page at a time.
    static_assert(sizeof(SmallItemBucket) * SmallItemHash::MAX_BUCKET_INDEX < cpu::PageSize, "Cannot fit all buckets on one page.");

    smBuckets = page_allocator.AllocatePageAs<SmallItemBucket>();
    util::span<SmallItemBucket> bucketSpan(smBuckets, SmallItemHash::MAX_BUCKET_INDEX);
    int index = 0;

    for (auto &bucket : bucketSpan)
    {
        new (&bucket) SmallItemBucket(index);
        //bucket.Prealloc(EMPTY_MAX);
        ++index;
    }

    // Safe to use kalloc_small for our List<LargePageMeta> objects.

    new (&freeLargeMetas) Stack<LargePageMeta>();
    new (&lgBucketMutex) thread::SpinLock();

    thread::LockGuard l(lgBucketMutex);

    for (size_t i = 0; i < LG_BUCKET_SIZE; ++i)
    {
        void *ptr = kalloc_small(sizeof(List<LargePageMeta>));
        lgBuckets[i] = new (ptr) List<LargePageMeta>();
    }

    kernel::arguments.CanKalloc = true;

    Debug::PrintF("EXIT: memory::init_kalloc()\r\n");
}

/********************************************************************************************************************/

static
void *kalloc_small(size_t sz)
{
#if MALLOC_CHECK_LEVEL >= 4
    if (sz > SmallItemHash::MAX_ITEM_SIZE)
        kpanic("kalloc() sent bad size to kalloc_small()");
#endif

    void *result = nullptr;

    for (size_t idx = SmallItemHash::IndexFromSize(sz); idx < SmallItemHash::MAX_BUCKET_INDEX && !result; ++idx)
    {
        // Tries for progressively larger buckets if the better fit doesn't have any space available.
        result = smBuckets[idx].Allocate();
    }

    return result;
}

/********************************************************************************************************************/

static void *kalloc_large(size_t sz)
{
#if MALLOC_CHECK_LEVEL >= 4
    if (sz <= SmallItemHash::MAX_ITEM_SIZE)
        kpanic("kalloc() sent bad size to kalloc_large()");
#endif

    //sz = paging::AlignCeiling(sz);
    size_t alignedSize = util::AlignCeiling<cpu::PageSize>(sz);
    size_t pageCnt = alignedSize / cpu::PageSize;

    if (pageCnt > 1)
    {
        // Need to finish writing the page allocator so we can allocate multiple blocks of pages first.
        kpanic("Request to kalloc() for more than a single page worth of memory!");
    }
    
    LargePageMeta *meta = freeLargeMetas.Pop();

    if (!meta)
    {
        void *ptr = kalloc_small(sizeof(LargePageMeta));

        if (ptr == nullptr)
            return nullptr; // Out of memory!

        meta = new (ptr) LargePageMeta();
    }

    //meta->PagePtr = page_allocator.AllocatePagesAs<void *>(pageCnt);
    meta->PagePtr = page_allocator.AllocatePageAs<void *>();
    ++kallocAllocatedPages;
    //Debug::PrintF("Allocated page for large bucket.\r\n");

    if (meta->PagePtr == nullptr)
    {
        // Out of memory!
        freeLargeMetas.Push(meta);
        return nullptr;
    }

    meta->PageCount = pageCnt;
    uint8_t idx = hash_ptr(meta->PagePtr);

    thread::LockGuard l(lgBucketMutex);
    lgBuckets[idx]->PushFront(meta);

    return meta->PagePtr;
}

/********************************************************************************************************************/

void *kalloc(size_t size)
{
    if (size == 0)
        return nullptr;

    if (!kernel::arguments.CanKalloc)
        kpanic("kalloc(): Called before initialized!\r\n");

    if (size <= SmallItemHash::MAX_ITEM_SIZE)
        return kalloc_small(size);

    return kalloc_large(size);
}

/********************************************************************************************************************/

void kfree(void *ptr)
{
    if (ptr == nullptr)
        return;

    if (!kernel::arguments.CanKalloc)
        kpanic("kfree(): Called before kalloc() initialized!\r\n");

    uintptr_t ip = reinterpret_cast<uintptr_t>(ptr);

    if (!(ip & ~cpu::PageMask))
    {
        size_t bucket = hash_ptr(ptr);

        thread::LockGuard l(lgBucketMutex);

        for (auto &meta : *lgBuckets[bucket])
        {
            if (meta.PagePtr == ptr)
            {
                lgBuckets[bucket]->Remove(&meta);

                page_allocator.ReleasePage(meta.PagePtr);
                meta.PagePtr = nullptr;
                --kallocAllocatedPages;
                //Debug::PrintF("Release page for large bucket.\r\n");

                freeLargeMetas.Push(&meta);
                return;
            }
        }
    }
    else
    {
        SmallItemSlab *smSlab = SmallItemSlab::GetSlabFromPtr(ptr);

        if (smSlab != nullptr)
            smBuckets[smSlab->getBucket()].Return(smSlab, ptr);
    }
}

/********************************************************************************************************************/
