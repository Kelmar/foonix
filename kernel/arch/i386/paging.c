/*************************************************************************/
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "kheap.h"
#include "paging.h"

/*************************************************************************/

#define PAGE_TAB_SIZE 1024 /* In entries */

/*************************************************************************/

/* Defined in kheap.c */
extern uintptr_t s_placement_address;

page_directory_entry_t *page_directory;
page_t *page_table;

/*************************************************************************/

void set_page(void *);

void handle_page_fault(struct regs *);

/*************************************************************************/

void init_paging(void)
{
    uint32_t addr;
    int i;

    page_directory = kmalloc_a(sizeof(page_directory_entry_t) * PAGE_TAB_SIZE);
    memset(page_directory, 0, sizeof(page_directory_entry_t) * PAGE_TAB_SIZE);

    page_table = kmalloc_a(sizeof(page_t) * PAGE_TAB_SIZE);
    memset(page_table, 0, sizeof(page_t) & PAGE_TAB_SIZE);

    // Setup the page directory entries.
    for (i = 0; i < PAGE_TAB_SIZE; ++i)
	page_directory[i].writable = 1;

    // Map first 4 megs
    for (addr = 0, i = 0; i < PAGE_TAB_SIZE; ++i, addr += 0x1000)
    {
	page_table[i].present = 1;
	page_table[i].writable = 1;
	page_table[i].addr = addr >> 12;
    }

    page_directory[0].page = ((uint32_t)page_table) >> 12;
    page_directory[0].present = 1;
    page_directory[0].writable = 1;
    
    set_isr_callback(0x0E, handle_page_fault);

    set_page(page_directory);

    kprintf("Paging enabled\n");
}

/*************************************************************************/

struct bitinfo_TYPE
{
    uint32_t bit;
    const char *name;
};

typedef struct bitinfo_TYPE bitinfo_t;

bitinfo_t pfbits[] =
{
    {0x01, "present"},
    {0x02, "readonly"},
    {0x04, "user"},
    {0x08, "reserved"},
    {0, 0} // terminator
};

/*************************************************************************/

void print_bitinfo(bitinfo_t *info, uint32_t bits)
{
    uint32_t u = bits;
    int i = 0;

    while (info[i].bit != 0)
    {
	if ((u & info[i].bit) == info[i].bit)
	{
	    putstr(info[i].name);

	    u &= ~info[i].bit;
	    if (u)
		putstr(" ");
	}

	++i;
    }
}

/*************************************************************************/

void handle_page_fault(struct regs *r)
{
    uint32_t addr = read_cr2();

    kprintf("Page fault: (");
    print_bitinfo(pfbits, r->err_code);
    kprintf(") %p\n", addr);
}

/*************************************************************************/

