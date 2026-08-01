/*************************************************************************/
/*
 * $Id: kheap.c 187 2011-08-27 16:27:33Z kfiresun $
 */
/*************************************************************************/
/*
 * A simple heap manager.
 *
 * This system is prone to problems if anything manages to write over our
 * magic numbers; which is a strong possibility if a program over/under
 * runs.
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "paging.h"
#include "kheap.h"
#include "klist.h"

/*************************************************************************/

#define HEAP_INDEX_SIZE		0x00020000
#define HEAP_MAGIC_SAFE		0x5AFEC0DE
#define HEAP_MAGIC_DEAD		0xDEADBEEF
#define HEAP_MIN_SIZE		0x00070000

/* Minimum size of holes in bytes */
#define HEAP_MIN_HOLE_SIZE	0x00000010

/*************************************************************************/

/* Defined in the linker script. */
extern uint32_t	end;

uintptr_t s_placement_address = (uintptr_t)&end;

/* Defined in paging.c */
extern page_directory_t *s_page_directory;

heap_t *kheap = 0;

/*************************************************************************/

struct heap_header_TYPE
{
    uint32_t magic;
    bool_t   is_hole;
    size_t   size;
};

typedef struct heap_header_TYPE heap_header_t;

struct heap_footer_TYPE
{
    uint32_t magic;
    heap_header_t *header;
};

typedef struct heap_footer_TYPE heap_footer_t;

struct heap_TYPE
{
    klist_t   *index;
    uintptr_t  start_addr; // Start of allocated space.
    uintptr_t  end_addr;   // End of allocated space.
    uintptr_t  max_addr;   // The max address heap can grow to.
    bool_t     supervisor; // Supervisor heap
    bool_t     readonly;   // Read-only heap
};

/*************************************************************************/
/**
 * Find the smallest memory hole that fits the given size.
 *
 * @param heap -- Heap to search
 * @param size -- Size to look for
 * @param align -- Set to true if the memory needs to be page aligned.
 *
 * @return (size_t)(-1) if no holes found.
 * @return Index into the heap index if a hole was found.
 */
static size_t find_smallest_hole(heap_t *heap, size_t size, bool_t align)
{
    size_t idx_count = klist_count(heap->index);
    size_t i;

    for (i = 0; i < idx_count; ++i)
    {
        heap_header_t *header = (heap_header_t *)klist_get_at(heap->index, i);

        if (align)
        {
            /* Align the page if possible. */
            uintptr_t loc = (uintptr_t)header;
            ssize_t hole_size;
            ssize_t off = 0;

            /* The data needs to be page aligned, not our header! */
            loc += sizeof(heap_header_t);

            if ((loc & PAGE_ALIGN_MASK) != 0)
            off = PAGE_SIZE - loc % PAGE_SIZE;

            hole_size = (ssize_t)header->size - off;

            if (hole_size >= (ssize_t)size)
            break; /* Still fits! */
        }
        else if (header->size >= size)
            break;
    }

    if (i >= idx_count)
        return (size_t)(-1);

    return i;
}

/*************************************************************************/

static void expand(heap_t *heap, size_t new_size)
{
    size_t old_sz = (heap->end_addr - heap->start_addr);
    size_t i;

    if (new_size < old_sz)
        return; // No need to expand

    if ((new_size & PAGE_ALIGN_MASK) != 0)
    {
        new_size &= PAGE_ALIGN_MASK;
        new_size += PAGE_SIZE;
    }

    ASSERT((heap->start_addr + new_size) < heap->max_addr, "Expanded heap size would exceed maximum allowed size.");

    for (i = old_sz; i < new_size; i += PAGE_SIZE)
    {
        page_t *page = get_page(heap->start_addr + i, true, s_page_directory);
        int flags = 0;

        flags |= heap->supervisor ? AFF_KERNEL : AFF_USER;
        flags |= heap->readonly ? AFF_RDONLY : AFF_RDWR;

        alloc_frame(page, flags);
    }

    heap->end_addr = heap->start_addr + new_size;
}

/*************************************************************************/

static size_t contract(heap_t *heap, size_t new_size)
{
    size_t old_sz = heap->end_addr - heap->start_addr;
    size_t i;

    if (new_size > old_sz)
        return old_sz; // No need to contract.

    if (new_size & PAGE_SIZE)
    {
        new_size &= PAGE_SIZE;
        new_size += PAGE_SIZE;
    }

    /* Don't contract too far */
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    for (i = old_sz - PAGE_SIZE; i > new_size; i -= PAGE_SIZE)
    {
        page_t *page = get_page(heap->start_addr + i, false, s_page_directory);
        free_frame(page);
    }

    heap->end_addr = heap->start_addr + new_size;
    return new_size;
}

/*************************************************************************/

static int heap_t_less_than(void *a, void *b)
{
    heap_header_t *ha = (heap_header_t *)a;
    heap_header_t *hb = (heap_header_t *)b;

    if (ha->size > hb->size)
        return 1;

    if (ha->size < hb->size)
        return -1;

    return 0;
}

/*************************************************************************/

heap_t *create_heap(
	uintptr_t start,
	uintptr_t end,
	size_t max,
	bool_t supervisor,
	bool_t readonly)
{
    ASSERT((start % PAGE_SIZE) == 0, "Start address not page aligned.");
    ASSERT((end % PAGE_SIZE) == 0, "End adress not page aligned.");

    heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));

    /* TODO: Fill in with specific placement allocator. */
    heap->index = klist_create(HEAP_INDEX_SIZE, &heap_t_less_than, NULL);

    /* Shift the start to where we can place data. */
    start += klist_size(heap->index);

    /* Page align it as well. */
    if ((start & PAGE_ALIGN_MASK) != 0)
    {
        start &= PAGE_ALIGN_MASK;
        start += PAGE_SIZE;
    }

    heap->start_addr = start;
    heap->end_addr = end;
    heap->max_addr = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    /* We start off with one large hole in the index. */
    heap_header_t *hole = (heap_header_t *)start;
    hole->size = end - start;
    hole->magic = HEAP_MAGIC_SAFE;
    hole->is_hole = true;

    klist_insert(heap->index, hole);

    return heap;
}

/*************************************************************************/

static void *alloc(heap_t *heap, size_t size, bool_t align)
{
    /* The total size of both ends of a heap block. */
    const size_t HEAP_ENDS_SIZE =
	sizeof(heap_header_t) + sizeof(heap_footer_t);

    size_t new_size = size + HEAP_ENDS_SIZE;
    size_t idx = find_smallest_hole(heap, new_size, align);

    heap_header_t *orig_hole_header;
    heap_header_t *block_header;
    heap_footer_t *block_footer;
    uintptr_t orig_hole_pos;
    size_t orig_hole_size;

    if (idx == (size_t)(-1))
    {
        // Couldn't find a hole large enough.
        size_t old_length = heap->end_addr - heap->start_addr;
        uintptr_t old_end_addr = heap->end_addr - heap->start_addr;
        size_t idx_count = klist_count(heap->index);
        size_t new_length;
        size_t value, i;

        expand(heap, old_length + new_size);
        new_length = heap->end_addr - heap->start_addr;

        /* Find the endmost header.  (In location, not size) */
        i = (size_t)(-1);
        for (value = idx = 0; idx < idx_count; ++idx)
        {
            uintptr_t tmp = (uintptr_t)klist_get_at(heap->index, idx);

            if (tmp > value)
            {
                value = tmp;
                i = idx;
            }
        }

        if (i == (size_t)(-1))
        {
            /* If we didn't find any headers, we need to add one. */
            heap_header_t *header = (heap_header_t *)old_end_addr;
            heap_footer_t *footer;

            header->magic = HEAP_MAGIC_SAFE;
            header->size = new_length - old_length;
            header->is_hole = true;

            footer = (heap_footer_t *)(old_end_addr + header->size -
            sizeof(heap_footer_t));
            footer->magic = HEAP_MAGIC_SAFE;
            footer->header = header;

            klist_insert(heap->index, header);
        }
        else
        {
            /* The last header needs adjusting */
            heap_header_t *header = (heap_header_t *)klist_get_at(heap->index, i);
            heap_footer_t *footer;

            header->size += new_length - old_length;

            footer = (heap_footer_t *)((uintptr_t)header +
            header->size - sizeof(heap_footer_t));
            footer->magic = HEAP_MAGIC_SAFE;
            footer->header = header;
        }

        /* Should now be enough space, recuse and return result. */
        return alloc(heap, size, align);
    }

    orig_hole_header = (heap_header_t *)klist_get_at(heap->index, idx);
    orig_hole_pos = (uintptr_t)orig_hole_header;
    orig_hole_size = orig_hole_header->size;

    if ((orig_hole_size - new_size) < (HEAP_ENDS_SIZE + HEAP_MIN_HOLE_SIZE))
    {
        /*
        * The hole isn't large enough to be split into a meaningful size.
        * So just use the whole thing.
        */
        size += orig_hole_size - new_size;
        new_size = orig_hole_size;
    }

    /* Page align the data */
    if (align && ((orig_hole_pos & PAGE_ALIGN_MASK) != 0))
    {
        /* What is this 0xFFF magic number?! */
        uintptr_t new_loc = orig_hole_pos + PAGE_SIZE - (orig_hole_pos & 0xFFF) - sizeof(heap_header_t);

        heap_header_t *hole_header = (heap_header_t *)orig_hole_pos;
        heap_footer_t *hole_footer = (heap_footer_t *)(new_loc - sizeof(heap_footer_t));

        hole_header->size = PAGE_SIZE - (orig_hole_pos & 0xFFF) - sizeof(heap_header_t);
        hole_header->magic = HEAP_MAGIC_SAFE;
        hole_header->is_hole = true;

        hole_footer->magic = HEAP_MAGIC_SAFE;
        hole_footer->header = hole_header;

        orig_hole_pos = new_loc;
        orig_hole_size -= hole_header->size;
    }
    else
    {
        /* Hole no longer needed, remove from index. */
        klist_remove(heap->index, idx);
    }

    /* Overwrite the original header. */
    block_header = (heap_header_t *)orig_hole_pos;
    block_header->magic = HEAP_MAGIC_SAFE;
    block_header->is_hole = false;
    block_header->size = new_size;

    /* And the footer */
    block_footer = (heap_footer_t *)(orig_hole_pos + sizeof(heap_header_t) + size);
    block_footer->magic = HEAP_MAGIC_SAFE;
    block_footer->header = block_header;

    /*
     * We may need to write a new hole after the allocated block.
     * We dot this only if the new hole would have a positive size.
     */
    if ((orig_hole_size - new_size) > 0)
    {
        heap_header_t *hole_header =
            (heap_header_t *)(orig_hole_pos + HEAP_ENDS_SIZE + size);
        heap_footer_t *hole_footer;

        hole_header->magic = HEAP_MAGIC_SAFE;
        hole_header->is_hole = true;
        hole_header->size = orig_hole_size - new_size;

        hole_footer = (heap_footer_t *)((uintptr_t)hole_header +
            orig_hole_size - new_size - sizeof(heap_footer_t));

        if ((uintptr_t)hole_footer < heap->end_addr)
        {
            hole_footer->magic = HEAP_MAGIC_SAFE;
            hole_footer->header = hole_header;
        }

        klist_insert(heap->index, hole_header);
    }

    return (void *)((uintptr_t)block_header + sizeof(heap_header_t));
}

/*************************************************************************/

static void free_heap_ptr(heap_t *heap, void *p)
{
    heap_header_t *header, *headtest;
    heap_footer_t *footer, *foottest;

    bool_t do_add = true; /* Assume we'll be adding the hole to the index. */

    if (p == NULL)
        return; /* Ignore NULLs */

    header = (heap_header_t *)((uintptr_t)p - sizeof(heap_header_t));
    footer = (heap_footer_t *)((uintptr_t)header + header->size - sizeof(heap_footer_t));

    if ((header->magic != HEAP_MAGIC_SAFE) || (footer->magic != HEAP_MAGIC_SAFE))
    {
        /* Invalid pointer, ignore. */
        return;
    }

    /*
    ASSERT(header->magic == HEAP_MAGIC_SAFE, "Invalid pointer in call to free_heap_ptr()");
    ASSERT(footer->magic == HEAP_MAGIC_SAFE, "Invalid pointer in call to free_heap_ptr()");
    */

    header->is_hole = true;

    /* Merge to the left? */
    foottest = (heap_footer_t *)((uintptr_t)header - sizeof(heap_footer_t));
    if ((foottest->magic == HEAP_MAGIC_SAFE) && foottest->header->is_hole)
    {
        size_t sz = header->size;

        /* Mark old header as dead */
        header->magic = HEAP_MAGIC_DEAD;

        header = foottest->header;

        footer->header = header;
        header->size += sz;

        /* The old footer is no longer needed, mark as dead. */
        foottest->magic = HEAP_MAGIC_DEAD;

        /* No need to add the header to the list again. */
        do_add = false;
    }

    /* Merge to the right? */
    headtest = (heap_header_t *)((uintptr_t)footer + sizeof(heap_footer_t));
    if ((headtest->magic == HEAP_MAGIC_SAFE) && headtest->is_hole)
    {
        size_t idx_count = klist_count(heap->index);
        size_t idx;

        header->size += headtest->size;

        /* Mark old footer as dead */
        footer->magic = HEAP_MAGIC_DEAD;

        footer = (heap_footer_t *)((uintptr_t)headtest + headtest->size -
            sizeof(heap_footer_t));

        /* Find the header we're merging with, so we can remove it. */
        for (idx = 0; idx < idx_count; ++idx)
        {
            if (klist_get_at(heap->index, idx) == headtest)
                break;
        }

        ASSERT(idx < idx_count, "Header not in heap index");

        klist_remove(heap->index, idx);
    }

    if (((uintptr_t)footer + sizeof(heap_footer_t)) == heap->end_addr)
    {
        /* We can shrink our heap. */
        size_t old_length = (uintptr_t)heap->end_addr - heap->start_addr;
        size_t new_length = contract(heap, (uintptr_t)header - 
            heap->start_addr);

        if ((header->size - (old_length - new_length)) > 0)
        {
            /* Hole still exists, so resize it. */
            header->size = old_length - new_length;

            /* Mark old footer as dead */
            footer->magic = HEAP_MAGIC_DEAD;

            footer = (heap_footer_t *)((uintptr_t)header + header->size -
            sizeof(heap_footer_t));

            footer->magic = HEAP_MAGIC_SAFE;
            footer->header = header;
        }
        else
        {
            /* Hole no longer exists.  Remove it from the index. */
            size_t idx, idx_count = klist_count(heap->index);

            header->magic = HEAP_MAGIC_DEAD;
            footer->magic = HEAP_MAGIC_DEAD;

            for (idx = 0; idx < idx_count; ++idx)
            {
                if (klist_get_at(heap->index, idx) == header)
                    break;
            }

            if (idx < idx_count)
                klist_remove(heap->index, idx);

            do_add = false; /* Paranoia */
        }
    }

    if (do_add)
        klist_insert(heap->index, header);
}

/*************************************************************************/
/*
 * "Allocate" a block of memory of a given number of bytes.
 *
 * This allocator is very simple and simply increments a pointer that starts
 * at the end of the loaded kernel's memory space and grows upwards (pointer
 * value increases) in memory.
 *
 * PARAMETERS:
 *    sz    - Size in bytes to allocate.
 *    align - Boolean indicating if the memory should be aligned in a 4KB
 *            boundary.
 *    phys  - Pointer to a pointer that receives the physical location of the
 *            memory allocated.
 *
 * RETURNS:
 *   A pointer to the allocated memory, which may be a pointer in a virtual
 * memory page.
 */
void *KMemAlloc(size_t sz, bool_t align, void **phys)
{
    void *rval, *p;

    if (kheap != NULL)
        rval = (void *)alloc(kheap, sz, align);
    else
    {
        if (align)
        {
            /* Align the allocation along a page boundary. */
            size_t asz = s_placement_address & PAGE_ALIGN_MASK;

            if (asz)
                s_placement_address += PAGE_SIZE - asz;
        }

        rval = (void *)s_placement_address;
        p = rval;

        s_placement_address += sz;
    }

    if (phys != NULL)
        *phys = p;

    return rval;
}

/*************************************************************************/

void *kmalloc_ap(size_t sz, void **phys)
{
    return KMemAlloc(sz, true, phys);
}

/*************************************************************************/

void *kmalloc_a(size_t sz)
{
    return KMemAlloc(sz, true, NULL);
}

/*************************************************************************/

void *kmalloc_p(size_t sz, void **phys)
{
    return KMemAlloc(sz, false, phys);
}

/*************************************************************************/

void *kmalloc(size_t sz)
{
    return KMemAlloc(sz, false, NULL);
}

/*************************************************************************/

void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    /* Not technically correct, but will do for now. */
    if (kheap != NULL)
        free_heap_ptr(kheap, ptr);
}

/*************************************************************************/

