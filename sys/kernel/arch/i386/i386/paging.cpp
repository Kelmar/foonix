/********************************************************************************************************************/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <kernel/arch/dconsole.h>

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
    //const size_t BYTE_BIT_SIZE = 8;

    //const size_t INT_BIT_SIZE = sizeof(uint32_t) * BYTE_BIT_SIZE;

    // Adjust an address for entry into a page/directory table.
    inline constexpr uintptr_t page_adjust_address(uintptr_t a) { return (a >> 12); }

    // Adjust a page/directory table entry to a real address.
    inline constexpr uintptr_t page_unadjust_address(uintptr_t a) { return (a << 12); }

    // Raise a number by a given power.
    //inline constexpr uint32_t pow(uint32_t base, uint32_t exp) { return exp == 0 ? 1 : base * pow(base, exp - 1); }

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

    extern "C" uintptr_t _kernel_start; // Start symbol of the kernel. (Defined by linker script)
    extern "C" uintptr_t _kernel_end;   // End symbol of the kernel. (Defined by linker script)

    extern "C" page_directory_t _boot_page_directory;
    //extern "C" page_t _boot_page_identity;
    //extern "C" page_t _boot_page_table1;
    
    /************************************************************************************************************/
}

/********************************************************************************************************************/
