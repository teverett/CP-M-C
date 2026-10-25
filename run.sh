#!/bin/sh
# Build the bare-metal i386 kernel and a system disk containing the utility
# COM programs, then boot them in QEMU on the serial console.
# Ctrl-A X quits QEMU.
set -e

cd "$(dirname "$0")"

QEMU386=${QEMU386:-qemu-system-i386}
PYTHON=${PYTHON:-python3}

make i386 utility-disk
"$PYTHON" tools/mkimage.py -o build/disks/utilities-system.img \
    -A build/disks/utilities-A.img

exec "$QEMU386" -nographic -nic none -m 16 -kernel build/i386/kernel.elf \
    -drive file=build/disks/utilities-system.img,format=raw,if=ide \
    -serial mon:stdio -serial file:build/disks/list.out \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
