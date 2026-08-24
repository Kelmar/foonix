/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

/********************************************************************************************************************/

namespace
{
    const int GDT_ENTRIES = 6;

    /*
     * A global descriptor table entry.
     */
    struct gdt_entry_t
    {
        uint16_t limit_low;   // Limit low part
        uint16_t base_low;    // Base address lo part
        uint8_t  base_middle; // Base address mid part
        uint8_t  access;      // Access flags
        uint8_t  granularity; // Limit granularity
        uint8_t  base_high;   // Base address high
    } __attribute__((packed));

    /*
     * Global descriptor table pointer.
     */
    struct gdt_ptr_t
    {
        uint16_t     limit; // Number of entries
        gdt_entry_t* base;  // Pointer to table itself.
    } __attribute__((packed));
}

extern "C" gdt_entry_t g_gdtEntries[GDT_ENTRIES];
extern "C" gdt_ptr_t g_gdtPtr;

extern "C" void load_gdt(gdt_ptr_t*);

#if 0

namespace
{
    void gdt_set_gate(size_t entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
    {
        g_gdtEntries[entry].base_low    = (base & 0xFFFF);
        g_gdtEntries[entry].base_middle = (base >> 16) & 0xFF;
        g_gdtEntries[entry].base_high   = (base >> 24) & 0xFF;

        g_gdtEntries[entry].limit_low   = (limit & 0xFFFF);
        g_gdtEntries[entry].granularity = (limit >> 16) & 0x0F;

        g_gdtEntries[entry].granularity |= granularity & 0xF0;
        g_gdtEntries[entry].access      = access;
    }
}

/********************************************************************************************************************/

void init_gdt(void)
{
    // This is a fairly basic GDT to get us started, we'll probably want to expand on this system later.

    g_gdtPtr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
    g_gdtPtr.base  = g_gdtEntries;

    gdt_set_gate(0, 0, 0, 0, 0); // NULL segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment (Kernel)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment (Kernel)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // Code segment (User)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // Data segment (User)

    // This is probably what's causing our triple fault
    load_gdt(&g_gdtPtr);
}

#endif

/********************************************************************************************************************/
