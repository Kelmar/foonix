/********************************************************************************************************************/

#include "multiboot.h"

/********************************************************************************************************************/

multiboot_t* g_multiboot_record;

/********************************************************************************************************************/

void terminal_pre_init(void);

void init_gdt(void);
void init_idt(void);
void* init_paging(void);

/********************************************************************************************************************/
/**
 * Pre init stage, called before static constructors.
 * 
 * The main purpose is to place the multiboot record pointer someplace where we can find it, as well as setup the GDT
 * and paging first thing.
 */
extern "C" void* stage1(multiboot_t* mbr)
{
    g_multiboot_record = mbr;

    terminal_pre_init();

    init_gdt();
    init_idt();

    return init_paging();
}

/********************************************************************************************************************/
