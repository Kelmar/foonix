/********************************************************************************************************************/
/*
 * x86 32-bit CPU instructions made available to C/C++ code.
 */
 /********************************************************************************************************************/
 
#include "asm.h"
#include "cpuflags.h"

/*
 * Halts execution of the system.
 *
 * void khalt(void);
 */
_LABEL(khalt)
    cli
.L_hlt:
    hlt
    pause
    jmp .L_hlt

/*
 * Reads the CPU counter.
 *
 * uint32_t read_cpu_counter(uint32_t *lo_value, uint32_t *hi_value);
 */
_LABEL(read_cpu_counter)
    movl 4(%esp), %ebx
    movl 8(%esp), %ecx

    rdtsc

    /* Check for NULL pointer */
    test %ebx, %ebx
    jz 1f
    movl %eax, (%ebx)
1:
    /* Check for NULL pointer */
    test %ecx, %ecx
    jz 1f
    movl %edx, (%ecx)
1:
    ret

/*
 * Returns the number of seconds currently in the RTC.
 * Used for timing.
 *
 * uint32_t read_rtc_second(void);
 */
_LABEL(read_rtc_second)
    xorl %eax, %eax
    outb %al, $0x40
    pause
    inb $0x41, %al
    ret

_LABEL(has_cpuid)
    pushf
    pop   %eax
    movl  %eax, %ecx
    xorl  $0x00200000, %eax
    pushl %eax
    popf
    pushf
    popl  %eax
    push  %ecx
    popf
    xor   %ecx, %eax
    jz    .L_no_cpuid
    movl  $1, %eax
    ret

.L_no_cpuid:
    xorl  %eax, %eax
    ret

/*
 * Reads the current time of day from the RTC.
 *
 * Value returned is the number of seconds since midnight.
 */
_LABEL(read_rtc_time)
    xorl %eax, %eax
    movl $60, %edx

    /* Read the hour */
    movb $4, %al
    outb %al, $0x40
    pause
    inb $0x41, %al
    mul %eax
    movl %eax, %ebx

    /* Read the minutes */
    xorl %eax, %eax
    movb $2, %al
    outb %al, $0x40
    pause
    inb $0x41, %al
    mul %eax
    addl %eax, %ebx

    /* Read the seconds */
    xorl %eax, %eax
    outb %al, $0x40
    pause
    inb $0x41, %al
    addl %ebx, %eax

    ret


/*
 * Provides a brief short delay, the length of the delay is highly dependent
 * on the speed of the computer the software is running on!
 *
 * This function is for creating delays when we need to wait on a hardware
 * response.
 */
_LABEL(soft_delay)
    movl $2000, %ecx
.L_loop:
    pause
    loop .L_loop
    ret

/*
 * Reboots the PC.
 *
 * int reboot(void);
 *
 * Returns 0xFFFFFFFF on failure.
 */
_LABEL(reboot)
    /* Flush the keyboard buffer */
    cli

.L_flush_top:
     movl $0x64, %edx
     inb %dx, %al
     movb %bl, %al
     andb $1, %al
     jz .L_flush_test

     movl $0x60, %edx
     inb %dx, %al
    
.L_flush_test:
    andb $2, %bl
    jnz .L_flush_top

    /* Send reboot command to keyboard.... */
    movl $0x60, %edx
    movl $0xFE, %eax
    outb %al, %dx

    /* If we got here, the computer didn't reboot.... */
    movl $0xFFFFFFFF, %eax
    ret
