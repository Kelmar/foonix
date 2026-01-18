/* The bootloader will look at this image and start execution at the symbol designated at the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final kernel image. */
SECTIONS
{
    /* Begin putting sections at 1 MiB, a conventional place for kernels to be loaded at by the bootloader. */
    . = 1M;

    /* Start of kernel marker */
    kernel_start = .; _kernel_start = .;  __kernel_start = .;

    .multiboot.data :
    {
        *(.multiboot.data)
    }

    .multiboot.text :
    {
        *(.multiboot.text)
    }

    .multiboot.strings :
    {
        *(.multiboot.strings)
    }

    . += 0xC0000000;

    .text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    /* End of kernel marker */
    kernel_end = .; _kernel_end = .; __kernel_end = .;
}