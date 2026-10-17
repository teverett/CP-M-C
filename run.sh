#!/bin/sh
# Build the bare-metal i386 kernel and its system disk, then boot them in
# QEMU on the serial console (same as `make run-i386`, as a standalone
# script). Ctrl-A X quits QEMU.
set -e

cd "$(dirname "$0")"

QEMU386=${QEMU386:-qemu-system-i386}

make i386 build/disks/system.img

mkdir -p build/disks
exec "$QEMU386" -nographic -nic none -m 16 -kernel build/i386/kernel.elf \
    -drive file=build/disks/system.img,format=raw,if=ide \
    -serial mon:stdio -serial file:build/disks/list.out \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
