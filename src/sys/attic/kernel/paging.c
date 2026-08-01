/*************************************************************************/
/*
 * $Id: paging.c 68 2010-09-05 17:03:21Z kfiresun $
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "kheap.h"
#include "paging.h"

/*************************************************************************/

#define PAGING_TABLE_SIZE 1024 /* In entries */

// Adjust an address for entry into a page/directory table.
#define PAGE_ADJUST_ADDRESS(A_) (((uintptr_t)A_) >> 12)

// Adjust a page/directory table entry to a real address.
#define PAGE_UNADJUST_ADDRESS(A_) (((uintptr_t)A_) << 12)

/*************************************************************************/

struct page_TYPE
{
    uint32_t present	: 1;  /* Set if this page is present. */
    uint32_t writable	: 1;  /* Set if this is a writable page. */
    uint32_t user	: 1;  /* Set if this is a user space page. */
    uint32_t writethu	: 1;
    uint32_t nocache	: 1;  /* Set if cache is disabled */
    uint32_t accessed	: 1;  /* Set by CPU if page has been accessed. */
    uint32_t dirty	: 1;  /* Set by CPU if page has been written to. */
    uint32_t reserved	: 1;  /* Always set to zero! */
    uint32_t global	: 1;
    uint32_t sys_bits	: 3;  /* For use by OS */
    uint32_t addr	: 20; /* Physical address (shifted right 12 bits) */
} __attribute__((packed));

/*
 * This is for a 4KByte directory entry.
 */
struct page_directory_entry_TYPE
{
    uint32_t present	: 1;  /* Set if this page is present. */
    uint32_t writable	: 1;  /* Set if this is a writable page. */
    uint32_t user	: 1;  /* Set if this is a user space page. */
    uint32_t writethu	: 1;
    uint32_t nocache	: 1;  /* Set if cache is disabled */
    uint32_t accessed	: 1;  /* Set by CPU if page has been accessed. */
    uint32_t reserved	: 1;  /* Always set to zero! */
    uint32_t pagesize	: 1;  /* 0 for 4KByte */
    uint32_t global	: 1;  /* Ignored, set to zero */
    uint32_t sys_bits	: 3;  /* For use by OS */
    uint32_t page_table	: 20; /* Page table address (shifted right 12 bits) */
} __attribute__((packed));

typedef struct page_directory_entry_TYPE page_directory_entry_t;

/*
 * Holds a table of page directory entries and other information.
 */
struct page_directory_TYPE
{
    page_directory_entry_t tables[PAGING_TABLE_SIZE];
} __attribute__((packed));

/*************************************************************************/

size_t phys_mem_size = (16 * 0x100000); /* Assume 16 MB for now. */

#define FRAME_UINT_BITSIZE (sizeof(uintptr_t) * 8)

#define INDEX_FROM_BIT(F_) (F_ / FRAME_UINT_BITSIZE)
#define OFFSET_FROM_BIT(F_) (F_ % FRAME_UINT_BITSIZE)

/* Set if paging has been initialized. */
bool_t s_paging_running = false;

/* Bitmap list of frames */
uintptr_t *s_frames;
size_t     s_frame_cnt;

page_directory_t *s_page_directory;

/* Defined in kheap.c */
extern uintptr_t s_placement_address;
extern heap_t *kheap;

/*************************************************************************/

/* Defined in cpu.S */
void set_pagedir(void *);
void flush_page_cache(void);
void copy_page(uintptr_t dest, uintptr_t source);

/*************************************************************************/

// Mark frame as used
static void set_frame(uintptr_t frame)
{
    if (frame > s_frame_cnt)
	kstop("Attempt to mark frame beyound number of frames.");

    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);

    s_frames[idx] |= (1 << off);
}

/*************************************************************************/

// Mark frame as available
static void clear_frame(uintptr_t frame)
{
    if (frame > s_frame_cnt)
    {
	// TODO: Log a warning
	//kstop("Attempt to clear frame beyound number of frames.");
	return;
    }

    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);

    s_frames[idx] &= ~(1 << off);
}

/*************************************************************************/

// Checks to see if a frame is available or not.
#if 0
static bool_t frame_is_available(uintptr_t frame)
{
    if (frame > s_frame_cnt)
    {
	// TODO: Log a warning
	return false;
    }

    uintptr_t idx = INDEX_FROM_BIT(frame);
    uintptr_t off = OFFSET_FROM_BIT(frame);

    return (s_frames[idx] & (1 << off));
}
#endif /* 0 */

/*************************************************************************/

// Find a free frame, returns UINTPTR_MAX if no frames are free.
static uintptr_t find_free_frame(void)
{
    uintptr_t i, j;

    for (i = 0; i < INDEX_FROM_BIT(s_frame_cnt); ++i)
    {
	if (s_frames[i] == UINTPTR_MAX)
	    continue;

	for (j = 0; i < FRAME_UINT_BITSIZE; ++j)
	{
	    uintptr_t toTest = 1 << j;

	    if (!(s_frames[i] & toTest))
		return (i * FRAME_UINT_BITSIZE + j);
	}
    }

    return UINTPTR_MAX;
}

/*************************************************************************/

void alloc_frame(page_t *page, int flags)
{
    (void)flags;

    if (page->addr != 0)
	return; /* Frame already allocated */
    else
    {
	uintptr_t idx = find_free_frame();

	if (idx == UINTPTR_MAX)
	    kstop("No free frames!");

	page->present = 1;
	page->writable = ((flags & AFF_RDWR) != 0) ? 1 : 0;
	page->user = ((flags & AFF_USER) != 0) ? 1 : 0;
	page->addr = idx;

	set_frame(idx); // Mark frame as used

	flush_page_cache();
    }
}

/*************************************************************************/

void free_frame(page_t *page)
{
    uintptr_t frame;

    if ((frame = page->addr) == 0)
	return;	// Frame already free
    else
    {
	clear_frame(frame);
	page->addr = 0;
    }
}

/*************************************************************************/
/**
 * Gets the page located at the given physical address.
 *
 * @param address -- The physical address to allocate
 * @param create  -- Set if the page should be created or not.
 * @param dir     -- (optional) The page directory to look in.
 *
 * @return The page located at the given physical address, or NULL if
 * the 'create' parameter is false.
 *
 * @note If the 'dir' parameter is NULL, then the kernel's page directory
 * will be scanned.
 */
page_t *get_page(uintptr_t address, bool_t create, page_directory_t *dir)
{
    page_t *rval = 0;

    address = PAGE_ADJUST_ADDRESS(address);

    uintptr_t dir_idx  = address / PAGING_TABLE_SIZE;
    uintptr_t page_idx = address % PAGING_TABLE_SIZE;

    if (dir == NULL)
	dir = s_page_directory; // So I don't have to keep linking it in. >_<

    if (dir->tables[dir_idx].present)
    {
	uintptr_t tmp = dir->tables[dir_idx].page_table;
	rval = (page_t *)PAGE_UNADJUST_ADDRESS(tmp);
    }
    else if (create)
    {
	rval = kmalloc_a(sizeof(page_t) * PAGING_TABLE_SIZE);

	memset(rval, 0, sizeof(page_t) * PAGING_TABLE_SIZE);

	dir->tables[dir_idx].page_table = PAGE_ADJUST_ADDRESS(rval);
	dir->tables[dir_idx].present = 1;
	dir->tables[dir_idx].writable = 1;

	flush_page_cache();
    }

    return rval ? &rval[page_idx] : 0;
}

/*************************************************************************/

/* Page fault error code bit names. */
bitinfo_t pfbits[] =
{
    {0x01, "present"},
    {0x02, "readonly"},
    {0x04, "user"},
    {0x08, "reserved"},
    {0, 0} // terminator
};

/*
 * Handle a page fault.
 */
void handle_page_fault(struct regs *r)
{
    uint32_t addr = read_cr2();

    kprintf("Page fault: 0x%08X (", r->err_code);
    kprint_bitinfo(pfbits, r->err_code);
    kprintf(") %p\n", addr);

    dump_regs(r);
    stack_trace(r->ebp);

    khalt();
}

/*************************************************************************/

page_t *clone_page_table(const page_t *source)
{
    void *phys;
    int i;

    page_t *rval = (page_t *)kmalloc_ap(sizeof(page_t) * PAGING_TABLE_SIZE,
        &phys);

    memset(rval, 0, sizeof(page_t) * PAGING_TABLE_SIZE);

    for (i = 0; i < PAGING_TABLE_SIZE; ++i)
    {
        if (!source[i].addr)
            continue;

        memcpy(&rval[i], &source[i], sizeof(page_t));

        /* TODO: Implement copy on write. */

        rval[i].addr = 0;

        alloc_frame(&rval[i], 0);

        copy_page(
            PAGE_UNADJUST_ADDRESS(rval[i].addr),
            PAGE_UNADJUST_ADDRESS(source[i].addr));
    }

    return rval;
}

/*************************************************************************/

page_directory_t *clone_directory(page_directory_t *source)
{
    page_t *newtab, *oldtab;
    void *phys;
    int i;

    page_directory_t *rval = (page_directory_t *)kmalloc_ap(
        sizeof(page_directory_t), &phys);
    memset(rval, 0, sizeof(page_directory_t));

    for (i = 0; i < PAGING_TABLE_SIZE; ++i)
    {
        if (!source->tables[i].page_table)
            continue;

        rval->tables[i] = source->tables[i];

        oldtab = (page_t *)PAGE_UNADJUST_ADDRESS(source->tables[i].page_table);
        newtab = (page_t *)PAGE_UNADJUST_ADDRESS(
            s_page_directory->tables[i].page_table);

        if (oldtab != newtab)
        {
            // Not a kernel page table, copy it.
            newtab = clone_page_table(oldtab);

            rval->tables[i].page_table = PAGE_ADJUST_ADDRESS(newtab);
        }
    }

    return rval;
}

/*************************************************************************/
/*
 * Initialize the paging system.
 */
void init_paging(multiboot_t *mbd)
{
    /*uint32_t addr;*/
    heap_t *heap;
    page_t *page;
    uintptr_t i;

    if ((mbd->flags & MB_FLAG_MEM) != 0)
    {
        kprintf("Lower memory available: %d kbytes\n", mbd->mem_lower);
        kprintf("Upper memory available: %d kbytes\n", mbd->mem_upper);

        phys_mem_size = mbd->mem_upper * 1024; // Convert to bytes
    }

    /*
    if ((mbd->flags & MB_FLAG_MMAP) != 0)
    {
        mb_memory_map_t *mmap = mbd->mmap_addr;

        while (mmap < (mbd->mmap_addr + mbd->mmap_length))
        {
            if (mmap->type == 1)
            {
                kprintf("Usable memory @ 0x%08X to 0x%08X\n",
                    mmap->base_addr,
                    (mmap->base_addr + mmap->length));
            }

            mmap += mmap->size + sizeof(uint32_t);
        }
    }
    */

    s_frame_cnt = phys_mem_size / PAGE_SIZE;
    s_frames = (uintptr_t *)kmalloc(s_frame_cnt / FRAME_UINT_BITSIZE);

    s_page_directory = kmalloc_a(sizeof(page_directory_t));
    memset(s_page_directory, 0, sizeof(page_directory_t));

    /* Setup the page directory entries. */
    for (i = 0; i < PAGING_TABLE_SIZE; ++i)
	s_page_directory->tables[i].writable = 1;

    /*
     * Initialize heap pages, we can't call alloc_frame() just yet because
     * they need to be mapped with the rest of the kernel.
     */
    for (i = 0; i < (uintptr_t)KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
    {
	get_page(i + KHEAP_START, true, s_page_directory);
    }

    heap = create_heap(
	KHEAP_START,
	KHEAP_START + KHEAP_INITIAL_SIZE,
	KHEAP_MAX_SIZE,
	true,
        false);

    /* Map kernel space. */
    i = 0;
    while (i < s_placement_address)
    {
	page = get_page(i, true, s_page_directory);
	alloc_frame(page, AFF_KERNEL | AFF_RDWR);

	i += PAGE_SIZE;
    }

    /* This seemed to fix the readonly issue we had with the heap and other code
     * so we were apparently not allocating enough pages.  Now we have a problem
     * where if we use the heap, newly allocated pages (after paging is enabled)
     * fault with no flags set, meaning the page isn't showing up to the CPU as
     * being marked present.....
     */
    page = get_page(i, true, s_page_directory);
    alloc_frame(page, AFF_KERNEL | AFF_RDWR);
    i += PAGE_SIZE;

    /*
     * Now it is safe to call alloc_frame() on the pages allocated for
     * our heap.
     */
    for (i = 0; i < KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
    {
	page = get_page(i + KHEAP_START, true, s_page_directory);
	alloc_frame(page, AFF_KERNEL | AFF_RDWR);
    }

    set_isr_callback(0x0E, handle_page_fault);

    /* Turn paging on! */
    set_pagedir(s_page_directory->tables);
    s_paging_running = true;
    kheap = heap;

    kprintf("Paging initialized\n");
}

/*************************************************************************/
