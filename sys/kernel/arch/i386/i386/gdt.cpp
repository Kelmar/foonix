/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

/********************************************************************************************************************/

namespace
{
    const int GDT_ENTRIES = 5;

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

    gdt_entry_t gdt_entries[GDT_ENTRIES];
    gdt_ptr_t   g_gdt_ptr;

    void gdt_set_gate(size_t entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
    {
        gdt_entries[entry].base_low    = (base & 0xFFFF);
        gdt_entries[entry].base_middle = (base >> 16) & 0xFF;
        gdt_entries[entry].base_high   = (base >> 24) & 0xFF;

        gdt_entries[entry].limit_low   = (limit & 0xFFFF);
        gdt_entries[entry].granularity = (limit >> 16) & 0x0F;

        gdt_entries[entry].granularity |= granularity & 0xF0;
        gdt_entries[entry].access      = access;
    }
}

extern "C" void load_gdt(gdt_ptr_t*);

/********************************************************************************************************************/

void init_gdt(void)
{
    g_gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
    g_gdt_ptr.base  = gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0); // NULL segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment (Kernel)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment (Kernel)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // Code sgement (User)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // Data segment (User)

    load_gdt(&g_gdt_ptr);
}

/********************************************************************************************************************/
