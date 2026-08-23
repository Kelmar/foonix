#!/bin/sh

PLATFORM="${1:-x86_64}"

QEMU="qemu-system-${PLATFORM}"
ISO="out/boot.${PLATFORM}.iso"

if [ ! -f "$ISO" ]; then
    echo "ISO not found: $ISO" >&2
    echo "Usage: $0 [platform]" >&2
    exit 1
fi

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
    -monitor telnet:127.0.0.1:5555,server,nowait \
    -serial mon:stdio \
    -cdrom "$ISO" | tee out/kernel.log

