ENTRY(_start)

__PAGE_SIZE = 0x1000;

/* TODO: Want to have one spot where we define this for everything. */
__VIRT_START = 0xC0000000;
/* __VIRT_START = 0; */

SECTIONS
{
    . = 1M;

    /* Create start marker */
    _kernel_phys_start = ABSOLUTE(.);

    .multiboot.data :
    {
        *(.multiboot.data)
    }

    .multiboot.text :
    {
        *(.multiboot.text)
    }

    . += __VIRT_START;

    .text ALIGN(__PAGE_SIZE) : AT (ADDR (.text) - __VIRT_START)
    {
        *(.text)
    }

    .rodata ALIGN(__PAGE_SIZE) : AT (ADDR (.rodata) - __VIRT_START)
    {
        *(.rodata)
    }

    .data ALIGN(__PAGE_SIZE) : AT (ADDR (.data) - __VIRT_START)
    {
        *(.data)
    }

    .bss ALIGN(__PAGE_SIZE) : AT (ADDR (.bss) - __VIRT_START)
    {
        *(COMMON)
        *(.bss)
    }

    /* Create kernel end marker */
    _kernel_end = ABSOLUTE(.);
}