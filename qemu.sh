#!/bin/sh

QEMU="qemu-system-x86_64"

#$QEMU -nographic -kernel kernel.elf -serial mon:stdio

rm -f out/qemu.log
rm -f out/kernel.log

# Some debugging options....
#-d trace:ahci_*,trace:ide_*,trace:cmd_identify,int,cpu_reset,guest_errors,invalid_mem \
#-d int,cpu,cpu_reset,guest_errors,invalid_mem \

#-d int,cpu_reset,guest_errors,invalid_mem \

# Start with GDB server
#-gdb tcp:0.0.0.0:1234 -S \

$QEMU \
    -d int,cpu_reset,guest_errors,invalid_mem \
    -D out/qemu.log \
    -s -S \
    -no-reboot \
    -no-shutdown \
    -serial mon:stdio \
    -cdrom out/boot.iso | tee out/kernel.log
