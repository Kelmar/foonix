#include <stdint.h>

typedef uint64_t page_entry_t;

typedef page_table_t page_entry_t[512];

// Defined in start.S
extern "C" page_table_t boot_pml4t;
extern "C" page_table_t boot_pdpt;
extern "C" page_table_t boot_pdt;
extern "C" page_table_t boot_pt;
