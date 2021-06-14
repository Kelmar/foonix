/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <kernel/flow.h>
#include <kernel/tty.h>

#include "atomic.h"
#include "cpu.h"
#include "cpudefs.h"
#include "multiboot.h"

/********************************************************************************************************************/

namespace
{
    const size_t PAGING_TABLE_SIZE = 1024; // In entries
    const size_t BYTE_BIT_SIZE = 8;

    const size_t INT_BIT_SIZE = sizeof(uint32_t) * BYTE_BIT_SIZE;

    // Adjust an address for entry into a page/directory table.
    inline constexpr uintptr_t page_adjust_address(uintptr_t a) { return (a >> 12); }

    // Adjust a page/directory table entry to a real address.
    inline constexpr uintptr_t page_unadjust_address(uintptr_t a) { return (a << 12); }

    // Raise a number by a given power.
    inline constexpr uint32_t pow(uint32_t base, uint32_t exp) { return exp == 0 ? 1 : base * pow(base, exp - 1); }

    /************************************************************************************************************/

    struct page_t
    {
        uint32_t present  : 1;  // Set if this page is present.
        uint32_t writable : 1;  // Set if this is a writable page.
        uint32_t user     : 1;  // Set if this is a user space page.
        uint32_t writethu : 1;
        uint32_t nocache  : 1;  // Set if cache is disabled
        uint32_t accessed : 1;  // Set by CPU if page has been accessed.
        uint32_t dirty    : 1;  // Set by CPU if page has been written to.
        uint32_t reserved : 1;  // Always set to zero!
        uint32_t global   : 1;
        uint32_t sys_bits : 3;  // For use by OS
        uint32_t addr     : 20; // Physical address (shifted right 12 bits)
    } __attribute__((packed));

    /*
     * This is for a 4KByte directory entry.
     */
    struct page_directory_entry_t
    {
        uint32_t present    : 1;  // Set if this page is present.
        uint32_t writable   : 1;  // Set if this is a writable page.
        uint32_t user       : 1;  // Set if this is a user space page.
        uint32_t writethu   : 1;
        uint32_t nocache    : 1;  // Set if cache is disabled
        uint32_t accessed   : 1;  // Set by CPU if page has been accessed.
        uint32_t reserved   : 1;  // Always set to zero!
        uint32_t pagesize   : 1;  // 0 for 4KByte
        uint32_t global     : 1;  // Ignored, set to zero
        uint32_t sys_bits   : 3;  // For use by OS
        uint32_t page_table : 20; // Page table address (shifted right 12 bits)
    } __attribute__((packed));

    /*
     * Holds a table of page directory entries and other information.
     */
    struct page_directory_t
    {
        page_directory_entry_t tables[PAGING_TABLE_SIZE];
    } __attribute__((packed));

    /************************************************************************************************************/

    /*
     * A bit map of allocated pages for the first 4MB.
     * 
     * We assume most systems have more memory than this these days.  (my 386 had this much RAM)
     */
    const size_t LOWER_1MB        = 1024 * 1024;
    const size_t LOWER_1MB_BITS   = LOWER_1MB / PAGE_SIZE;
    const size_t LOWER_1MB_BYTES  = LOWER_1MB_BITS / 8;
    const size_t LOWER_1MB_DWORDS = LOWER_1MB_BYTES / sizeof(uint32_t);

    const size_t LOWER_4MB        = 1024 * 1024 * 4;
    const size_t LOWER_4MB_BITS   = LOWER_4MB / PAGE_SIZE;
    const size_t LOWER_4MB_BYTES  = LOWER_4MB_BITS / 8;
    const size_t LOWER_4MB_DWORDS = LOWER_4MB_BYTES / sizeof(uint32_t);

    union
    {
        uint8_t  b_map[LOWER_4MB_BYTES];
        uint32_t i_map[LOWER_4MB_DWORDS];
    } s_low_bitmap;

    /************************************************************************************************************/

    //static void mark_reserved_page(uintptr_t page, bool set)
    //{
    //    uint32_t p = page_adjust_address(page);
    //    uint32_t idx = (p >> 5);               // Divide by 32
    //    uint32_t bit = 1 << (p & 0x0000001F);  // Shift 1 left by remainder 32

    //    for (;;)
    //    {
    //        uint32_t map = s_low_bitmap.i_map[idx];
    //        uint32_t nv = set ? (map | bit) : (map & ~bit);

    //        if (map == nv)
    //            return; // Value already set

    //        if (atomic_cmpset_uint32(&s_low_bitmap.i_map[p], map, nv))
    //            return;
    //    }
    //}

    /************************************************************************************************************/
}

/********************************************************************************************************************/
/**
 * Allocates one of our reserved pages below 4MB
 */
void* alloc_reserved_page(void)
{
    for (;;)
    {
        size_t p = LOWER_1MB_DWORDS;
        uint32_t map = s_low_bitmap.i_map[p];

        while (p < LOWER_4MB_DWORDS && map == 0xFFFFFFFF)
        {
            ++p;
            map = s_low_bitmap.i_map[p];
        }

        if (p >= LOWER_4MB_DWORDS)
            break;

        for (uint32_t i = 0; i < 32; ++i)
        {
            uint32_t bit = 1 << i;

            if (!(map & bit))
            {
                uint32_t nv = map | bit;
                bool res = atomic_cmpset_uint32(&s_low_bitmap.i_map[p], map, nv);

                if (res)
                {
                    uintptr_t phys = p * 32 + i;
                    return (void*)page_unadjust_address(phys);
                }
            }
        }
    }

    //panic("Out of reserved memory!");
    printf("Out of reserved memory?!\n");

    return nullptr;
}

/********************************************************************************************************************/
/**
 * Returns a reserved page back to the pool.
 */
void release_reserved_page(void* page)
{
    size_t x = (size_t)page;

    if (x < LOWER_1MB)
        panic("Cannot release pages lower than 1MB!\n");

    if (x > LOWER_4MB)
        panic("Out of range for reserved page!\n");
    
    uint32_t p = page_adjust_address((uintptr_t)page);
    uint32_t idx = (p >> 5);               // Divide by 32
    uint32_t bit = 1 << (p & 0x0000001F);  // Shift 1 left by remainder 32

    for (;;)
    {
        uint32_t map = s_low_bitmap.i_map[idx];

        if (map & bit)
        {
            uint32_t nv = map & ~bit;
            bool res = atomic_cmpset_uint32(&s_low_bitmap.i_map[idx], map, nv);

            if (res)
                break; // Bit updated!
        }
        else
            break; // Memory already released!
    }
}

/********************************************************************************************************************/

void* init_paging(void)
{
    // Reserve the first MiB right off, we'll need this for real mode stuff later.   (E.g. BIOS calls)
    memset(s_low_bitmap.b_map, 0xFF, LOWER_1MB_BYTES);
    memset(s_low_bitmap.b_map + LOWER_1MB_BYTES, 0, LOWER_4MB_BYTES - LOWER_1MB_BYTES);

    if (g_multiboot_record && g_multiboot_record->flags & MB_FLAG_MEM)
    {
        printf("MemLower: %u KiB\n", g_multiboot_record->mem_lower);
        printf("MemUpper: %u KiB\n", g_multiboot_record->mem_upper);

        //if (g_multiboot_record->flags & MB_FLAG_MMAP)
        //{
        //    for (uint32_t i = 0; i < g_multiboot_record->mmap_length; ++i)
        //    {
        //        mb_memory_map_t* rec = &g_multiboot_record->mmap_addr[i];

        //        if ((rec->length > 0) & (rec->type == 1))
        //        {
        //            printf("ENTRY %u: ", i);
        //            terminal_write64(rec->base_addr);
        //            printf(", ");
        //            terminal_write64(rec->length);
        //            printf(", %u\n", rec->type);
        //        }
        //    }
        //}
        //else
        //    printf("No memory map!\n");
    }
    else
        panic("TODO: Probe for available memory");
    
    return nullptr;
}

/********************************************************************************************************************/
