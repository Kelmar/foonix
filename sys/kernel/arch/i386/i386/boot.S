# Setting up the multiboot header - see GRUB dogs for details.
.set ALIGN,     1<<0                # Align laoded modules on page boundaries
.set MEMINFO,   1<<1                # Provide memory map
.set FLAGS,     ALIGN | MEMINFO     # This is the multiboot 'flag' field
.set MAGIC,     0x1BADB002          # 'magic number' lets bootloader find header
.set CHECKSUM,  -(MAGIC + FLAGS)    # Checksum required

# Multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve initial kernel stack space.
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    cli

    # Setup the stack
    mov   $stack_top, %esp

    cmp   $0x2BADB002, %eax
    jne   .L_no_multiboot

    push  %ebx   # Multiboot data structure.
    call  stage1 # Initialize CPU first

    # EAX should point to where our new stack is located at.
    #movl  %eax, %esp

    # Call static constructors
    call  _init

    # Call the kernel proper
    call  kmain
    jmp   .L_halt

.L_no_multiboot:
    movl  $0x000B8000, %ecx
    movb  $0x58, (%ecx) # Write an 'X' denoting an error

.L_halt:
    # Halt machine.
    cli
.L_lock:
    hlt
    pause
    jmp   .L_lock
    
.size _start, . - _start
