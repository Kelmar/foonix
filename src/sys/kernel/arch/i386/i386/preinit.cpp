/********************************************************************************************************************/

#include "multiboot.h"

#include <kernel/arch/dconsole.h>

#include "cpu.h"

/********************************************************************************************************************/

//multiboot_t* g_multiboot_record;

/********************************************************************************************************************/

void init_gdt(void);
void init_idt(void);
void init_timer(void);
void* init_paging(void);

/********************************************************************************************************************/
/**
 * Pre init stage, called before static constructors.
 * 
 * The main purpose is to place the multiboot record pointer someplace where we can find it, as well as setup the GDT
 * and paging first thing.
 */
extern "C" void* old_preinit()
{
    //g_multiboot_record = mbr;

    //bochs_breakpoint();
    //init_gdt();

    //bochs_breakpoint();
    //init_idt();

    //bochs_breakpoint();
    //init_timer();

    //bochs_breakpoint();
    //init_paging();

    // TODO: Return pointer to a new dynamic stack.
    return nullptr;
}

/********************************************************************************************************************/
