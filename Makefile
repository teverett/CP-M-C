CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -O2 -g
PYTHON  ?= python3

SRC = src/host.c src/z80/host_z80.c src/machine.c src/bios.c src/bdos.c \
      src/ccp.c src/8080/i8080.c src/z80/z80.c

all: build/cpm build/ref/ccp.bin build/ref/bdos.bin z80 i386

build/cpm: $(SRC) src/cpm.h src/8080/i8080.h src/plat.h src/geom.h src/machine.h \
           src/host.h src/z80/z80.h src/z80/z80_ports.h
	@mkdir -p build
	$(CC) $(CFLAGS) -DCPM_HOSTED -Isrc/8080 -Isrc/z80 -o $@ $(SRC)

# Reference binaries: the original assembly, at the 56K CP/M 2.2 addresses
build/ref/ccp.bin: orig/ccp.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0dc00h -o $@ -s build/ref/ccp.sym $<

build/ref/bdos.bin: orig/bdos.asm tools/asm80.py
	@mkdir -p build/ref
	$(PYTHON) tools/asm80.py -D origin=0e400h -o $@ -s build/ref/bdos.sym $<

test: all
	$(PYTHON) tests/run_tests.py

clean:
	rm -rf build

.PHONY: all test clean z80 i386 run-i386 check-i386 iso

# --- native Z80 build (SDCC) -------------------------------------------
# The same C compiled to Z80 code.  Memory map: TPA 0100-9FFF, CCP data at
# A000, BDOS entry page A800, compiled code from A900.
SDCC    ?= sdcc
SDAS    ?= sdasz80
Z80SRC   = z80/native z80/plat_z80 bios bdos ccp
Z80OBJ   = build/z80/crt0.rel $(addprefix build/z80/,$(addsuffix .rel,$(notdir $(Z80SRC))))
SDFLAGS  = -mz80 --std-c99 --opt-code-size

z80: build/z80/cpm.bin

build/z80/crt0.rel: src/z80/crt0.s
	@mkdir -p build/z80
	cd build/z80 && $(SDAS) -plosgff crt0.rel ../../src/z80/crt0.s

Z80DEPS = src/cpm.h src/plat.h src/geom.h src/z80/z80_ports.h

build/z80/%.rel: src/%.c $(Z80DEPS)
	@mkdir -p build/z80
	cd build/z80 && $(SDCC) $(SDFLAGS) -c ../../src/$*.c -o $*.rel

build/z80/%.rel: src/z80/%.c $(Z80DEPS)
	@mkdir -p build/z80
	cd build/z80 && $(SDCC) $(SDFLAGS) -I../../src -c ../../src/z80/$*.c -o $*.rel

build/z80/cpm.ihx: $(Z80OBJ)
	cd build/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0xA900 \
	    --data-loc 0xF800 $(notdir $(Z80OBJ)) -o cpm.ihx

build/z80/cpm.bin: build/z80/cpm.ihx
	$(PYTHON) tools/ihx2bin.py $< $@

# --- bare-metal i386 build (run under QEMU) -------------------------------
I386CC    ?= i686-elf-gcc
I386AS    ?= i686-elf-as
I386OBJDUMP ?= i686-elf-objdump
QEMU386   ?= qemu-system-i386
# The kernel has no IDT and never enables SSE/x87, so it must use general
# registers only; do not rely on the compiler's default -march (Ubuntu's
# cross compiler defaults to one that emits SSE and the guest then resets).
I386FLAGS  = -m32 -march=i686 -mgeneral-regs-only -ffreestanding -fno-pie \
             -fno-stack-protector -fcf-protection=none \
             -fno-asynchronous-unwind-tables -O2 -Wall -Wextra \
             -Isrc -Isrc/i386 -Isrc/8080
I386SRC    = machine bios bdos ccp 8080/i8080
I386OBJ    = build/i386/boot.o \
             $(addprefix build/i386/,$(addsuffix .o,$(notdir $(I386SRC)))) \
             build/i386/plat_i386.o build/i386/rt.o

i386: build/i386/kernel.elf

# --- bootable ISO (GRUB/El Torito, BIOS boot) for Proxmox/QEMU/VirtualBox --
# The CP/M disks are not on the ISO: attach build/disks/system.img (made by
# tools/mkimage.py) as a separate IDE/SATA drive, and boot the ISO as the CD.
GRUBMKRESCUE ?= grub-mkrescue

iso: build/cpm.iso

build/cpm.iso: build/i386/kernel.elf src/i386/grub.cfg
	@mkdir -p build/iso/boot/grub
	cp build/i386/kernel.elf build/iso/boot/kernel.elf
	cp src/i386/grub.cfg build/iso/boot/grub/grub.cfg
	$(GRUBMKRESCUE) -o $@ build/iso

build/i386/boot.o: src/i386/boot.s
	@mkdir -p build/i386
	$(I386AS) $< -o $@

I386DEPS = src/cpm.h src/plat.h src/geom.h src/machine.h src/8080/i8080.h

build/i386/%.o: src/%.c $(I386DEPS)
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/%.o: src/8080/%.c $(I386DEPS)
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/plat_i386.o: src/i386/plat_i386.c src/i386/boot_hdr.h src/cpm.h src/plat.h
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/rt.o: src/i386/rt.c
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/kernel.elf: $(I386OBJ) src/i386/link.ld
	$(I386CC) -T src/i386/link.ld -o $@ -ffreestanding -nostdlib -no-pie -Wl,--build-id=none \
	    $(I386OBJ) -lgcc

# Fail if the kernel contains SSE/MMX/x87 instructions (it would reset in QEMU)
check-i386: build/i386/kernel.elf
	@if $(I386OBJDUMP) -d $< | grep -Eq '%(xmm|mm[0-7]|st)|\b(fld|fst|fild|fistp|movaps|movups|movdqa|movdqu)'; then \
	    echo "kernel.elf uses SSE/x87 instructions"; exit 1; \
	else echo "kernel.elf: general registers only"; fi

# Interactive CP/M on QEMU's serial console (Ctrl-A X quits).  Blank formatted
# disks are created in build/disks/{A,B,C}.img if missing; put your own
# images (see tools/mkdisk.py) there to use them.
build/disks/A.img:
	@mkdir -p build/disks
	$(PYTHON) tools/mkdisk.py 0 $@
build/disks/B.img:
	@mkdir -p build/disks
	$(PYTHON) tools/mkdisk.py 1 $@
build/disks/C.img:
	@mkdir -p build/disks
	$(PYTHON) tools/mkdisk.py 2 $@

# The system disk is rebuilt on every run so that it picks up the images.
run-i386: build/i386/kernel.elf build/disks/A.img build/disks/B.img build/disks/C.img
	$(PYTHON) tools/mkimage.py -o build/disks/system.img \
	    -A build/disks/A.img -B build/disks/B.img -C build/disks/C.img
	$(QEMU386) -nographic -nic none -m 16 -kernel build/i386/kernel.elf \
	    -drive file=build/disks/system.img,format=raw,if=ide \
	    -serial mon:stdio -serial file:build/disks/list.out \
	    -device isa-debug-exit,iobase=0xf4,iosize=0x04
