/*************************************************************************/
/*
 * This is our initial boot loader after stage 0 or 1.  We get called from 
 * evertyhign here, EFI, multiboot and so on.
 *
 * This file establishes a few conventions to make sure we preseve some information:
 *
 * 1) We need to preserve the initial contents of EAX, which for multiboot has the
 *    multiboot header that we'll want to parse in the C code.
 */
/*************************************************************************/

# Setting up the multiboot header - see GRUB docs for details.
.set ALIGN,     1<<0                # Align laoded modules on page boundaries
.set MEMINFO,   1<<1                # Provide memory map
.set FLAGS,     ALIGN | MEMINFO     # This is the multiboot 'flag' field
.set MAGIC,     0x1BADB002          # 'magic number' lets bootloader find header
.set CHECKSUM,  -(MAGIC + FLAGS)    # Checksum required

.set TEXTADDR,  0x000B8000          # Physical location of VGA text mode
.set HIGHADDR,  0xC0000000

/*************************************************************************/

# Remap the higher half address to the physical address
#define MAP2PHYS(X_) X_ - HIGHADDR

#ifdef _USE_BOCHS
#define breakpoint xchgw %bx, %bx
#define BOCHS_PORT 0xE9
#else
#define breakpoint
#endif

/*************************************************************************/

# Multiboot header
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*************************************************************************/

.section .multiboot.strings, "aw"
.align 4
/*                            0         1         2          */
/*                            012345678901234567890123456789 */
.D_greeting:         .string "Loading FooNIX....            "
.D_no_multiboot_err: .string "Multiboot not detected.       "
.D_kernel_too_big:   .string "Kernel too big to map.        "
.D_preinit_start:    .string "Begining preinit now....      "

/*************************************************************************/

# Reserve initial kernel stack space.
.section .bootstrap_stack, "aw", @nobits
    .align 16
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

/*************************************************************************/

# Area for bootstrap page directory and page tables.
.section .bss, "aw", @nobits
    .align 4096
    .global boot_page_directory
boot_page_directory:
    .skip 4096

# Identity map for the first 4MB
    .global boot_page_identity
boot_page_identity:
    .skip 4096

# Map for the actual kernel pages.
    .global boot_page_table1
boot_page_table1:
    .skip 4096

/*************************************************************************/

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
    cli

    # Setup a temporary stack
    mov  $MAP2PHYS(stack_top), %esp
    mov  $MAP2PHYS(stack_top), %ebp
   
    movl $.D_greeting, %esi
    call MAP2PHYS(.L_write_string)

    # Make sure we have multiboot
    cmp   $0x2BADB002, %eax
    jne   MAP2PHYS(.L_no_multiboot)

    movl $1, MAP2PHYS(g_BootMethod)  # Indicate multiboot startup
    movl %ebx, MAP2PHYS(g_Multiboot) # Preserve the multiboot pointer

    call MAP2PHYS(.L_init_page_dir)
    call MAP2PHYS(.L_load_identity_map)
    call MAP2PHYS(.L_load_kernel_map)

    # Load the page directory into CR3
    movl $MAP2PHYS(boot_page_directory), %eax
    movl %eax, %cr3

    # Turn paging on
    movl %cr0, %eax
    orl  $0x80010000, %eax
    movl %eax, %cr0

    breakpoint

    # Jump to higher half address
    lea 1f, %eax
    jmp *%eax

.section .text
1:
    # From this point on we don't need to remap our calls to physical memory.

    # Remap the stack to virtual memory space.
    movl $stack_top, %esp
    movl $stack_top, %ebp

    movl $.D_preinit_start, %esi
    call .L_write_string

    call  preinit # Initialize CPU & VM first

    # Call static constructors
    call  _init

    # Call the kernel proper
    call  kmain
    cli # Disable interrupts if they're still active
    jmp   .L_halt

.L_no_multiboot:
    movl  $.D_no_multiboot_err, %eax
    call  .L_write_string

.L_halt:
    # Halt machine.
    hlt
    pause
    jmp   .L_halt

/*************************************************************************/

.L_init_page_dir:
    # Blank out the page directory
    cld
    movl $1024, %ecx
    xorl %eax, %eax
    movl $(MAP2PHYS(boot_page_directory)), %edi
    rep stosl

    # Load our entries into the page directory
    movl $(MAP2PHYS(boot_page_identity) + 0x003), MAP2PHYS(boot_page_directory) + 0

    # Compute the entry # where to put the kernel.
    movl $HIGHADDR, %eax

    # Normally we'd shift right by 22, but we need to also multiply by 4, save ourselves
    # some extra effort and only shift right by 20 bits
    shrl $20, %eax
    addl $MAP2PHYS(boot_page_directory), %eax

    movl $(MAP2PHYS(boot_page_table1) + 0x003), (%eax)

    ret

/*************************************************************************/
/*
 * Maps the given pages into the supplied page table.
 *
 * Parameters:
 * ecx - Number of pages to map (must be 4096 or less)
 * esi - Address of first physical page to map
 * edi - Physical address of the page table to load.
 */

.L_map_pages:
    push %edi
    push %esi
    push %ecx
    push %edx

.L_map_pages_loop:
    # Load page entry

    movl %esi, %edx
    orl  $0x003, %edx # Set page to be read/write and present.
    movl %edx, (%edi)

    # Move to next page
    addl $4096, %esi

    # Move to next entry
    addl $4, %edi

    loop .L_map_pages_loop

    pop %edx
    pop %ecx
    pop %esi
    pop %edi
    ret

/*************************************************************************/
/*
 * This simply identity maps the first 4MB of physical memory into the same
 * location in our boot page tables.  This is how the rest of the kernel
 * expects these pages to be mapped anyhow.
 */
.L_load_identity_map:
    movl $1024, %ecx
    movl $MAP2PHYS(boot_page_identity), %edi
    movl $0, %esi

    call .L_map_pages    
    ret

/*************************************************************************/
/*
 * This function tries to actually find our kernel in physical memory space
 * and makes an attempt to map it into the HIGHADDR.
 */
.L_load_kernel_map:
    # Blank out the kernel map pages.
    cld
    movl $1024, %ecx
    xorl %eax, %eax
    movl $(MAP2PHYS(boot_page_table1)), %edi
    rep stosl

# What follows is from osdev.org

    movl $MAP2PHYS(boot_page_table1), %edi
    movl $0, %esi
    movl 1024, %ecx

2:
    # Only map the kernel
    cmpl $_kernel_start, %esi
    jl 3f
    cmpl $MAP2PHYS(_kernel_end), %esi
    jge 4f

    # Map physical address as present and writable.
    movl %esi, %edx
    orl $0x003, %edx
    movl %edx, (%edi)

3:
    # Size of page is 4096 bytes
    addl $4096, %esi
    addl $4, %edi
    loop 2b

4:

    ret
 #if 0
# My broken version which I'll take another stab at later

.L_load_kernel_map:
    # Blank out the kernel map pages.
    cld
    movl $1024, %ecx
    xorl %eax, %eax
    movl $(MAP2PHYS(boot_page_table1)), %edi
    rep stosl

    # Get the number of pages to load.
    movl $_kernel_start, %esi
    movl %esi, %edx
    movl $MAP2PHYS(_kernel_end), %ecx
    subl %edx, %ecx

    movl %ecx, %eax
    andl $0x00000FFF, %eax
    jz 2f

    # We don't end on a 4096 mark, make sure we map enough pages after division.
    addl $4096, %ecx
    
2:
    shrl $12, %ecx

    cmpl $1024, %ecx
    jle 3f

    movl $.D_kernel_too_big, %eax
    call MAP2PHYS(.L_write_string)

3:
    breakpoint

    ;movl %esi, %eax
    ;movl $0xC0000000, %eax
    ;shrl $20, %eax
    movl $MAP2PHYS(boot_page_table1), %edi
    ;addl %eax, %edi

    # Map the kernel
    call .L_map_pages

    ret
 #endif

/*************************************************************************/
/*
 * Prints a null terminated string to the VGA color text area.
 * For BOCHS we also write to the debug port.
 *
 * Parameters:
 * %esi - Location of string to print in physical memory
 *
 * This function is really primitive.  It doesn't account for
 * the cursor location, it doesn't know if we're in text mode
 * or not it just blindly writes to the first address in the
 * video memory in the hopes that it's visible to the end user.
 */ 
.L_write_string:
    pushl %esi
    pushl %eax
    pushl %edx
    pushl %edi

    movl $TEXTADDR, %edi

#ifdef _USE_BOCHS
    movl $BOCHS_PORT, %edx
#endif

    movb $0x07, %ah     # Load white on black color

.L_write_string_loop:
    movb (%esi), %al
    cmp $0, %al
    jz .L_write_string_done

#ifdef _USE_BOCHS
    outb %al, %dx       # Write character to debug port
#endif

    movw %ax, (%edi)    # Write character and color
    add $2, %edi
    inc %esi
    jmp .L_write_string_loop

.L_write_string_done:

#ifdef _USE_BOCHS
    # Write a NL/CR to the debug port
    movb $10, %al
    outb %al, %dx
    movb $13, %al
    outb %al, %dx
#endif

    popl %edi
    popl %edx
    popl %eax
    popl %esi
    ret


/*************************************************************************/
