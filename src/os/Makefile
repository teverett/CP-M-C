# Builds the OS itself: the hosted test binary, the native Z80 image, the
# bare-metal i386 kernel, and the artifacts (ISO, disk images) built from it.
# Included from the top-level Makefile, so all paths here are relative to
# the repo root, not to this file's directory.

SRC = src/os/host.c src/os/z80/host_z80.c src/os/machine.c src/os/bios.c src/os/bdos.c \
      src/os/ccp.c src/os/8080/i8080.c src/os/z80/z80.c

build/cpm: $(SRC) src/os/cpm.h src/os/8080/i8080.h src/os/plat.h src/os/geom.h src/os/machine.h \
           src/os/host.h src/os/z80/z80.h src/os/z80/z80_ports.h
	@mkdir -p build
	$(CC) $(CFLAGS) -DCPM_HOSTED -Isrc/os/8080 -Isrc/os/z80 -o $@ $(SRC)

.PHONY: z80 i386 run-i386 check-i386 iso fsimage z80-ed-com z80-util-coms utility-disk

# --- native Z80 build (SDCC) -------------------------------------------
# The same C compiled to Z80 code.  Memory map: TPA 0100-9FFF, CCP data at
# A000, BDOS entry page A800, compiled code from A900.
SDCC    ?= sdcc
SDAS    ?= sdasz80
Z80SRC   = z80/native z80/plat_z80 bios bdos ccp
Z80OBJ   = build/z80/crt0.rel $(addprefix build/z80/,$(addsuffix .rel,$(notdir $(Z80SRC))))
SDFLAGS  = -mz80 --std-c99 --opt-code-size

z80: build/z80/cpm.bin

build/z80/crt0.rel: src/os/z80/crt0.s
	@mkdir -p build/z80
	cd build/z80 && $(SDAS) -plosgff crt0.rel ../../src/os/z80/crt0.s

Z80DEPS = src/os/cpm.h src/os/plat.h src/os/geom.h src/os/z80/z80_ports.h

build/z80/%.rel: src/os/%.c $(Z80DEPS)
	@mkdir -p build/z80
	cd build/z80 && $(SDCC) $(SDFLAGS) -c ../../src/os/$*.c -o $*.rel

build/z80/%.rel: src/os/z80/%.c $(Z80DEPS)
	@mkdir -p build/z80
	cd build/z80 && $(SDCC) $(SDFLAGS) -I../../src -c ../../src/os/z80/$*.c -o $*.rel

build/z80/cpm.ihx: $(Z80OBJ)
	cd build/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0xA900 \
	    --data-loc 0xF800 $(notdir $(Z80OBJ)) -o cpm.ihx

build/z80/cpm.bin: build/z80/cpm.ihx
	$(PYTHON) tools/ihx2bin.py $< $@

# Standalone Z80 COM utility. Its entry code and BDOS gate are separate from
# the resident OS image and execute from the normal 0100h TPA address.
z80-ed-com: build/util/z80/ED.COM

UTIL_NAMES = ed dump load pip stat submit mac
UTIL_COM_FILES = $(addprefix build/util/z80/,$(addsuffix .COM,ED DUMP LOAD PIP STAT SUBMIT MAC BASIC BASICBAS BASICINT))

z80-util-coms: $(UTIL_COM_FILES)

build/util/z80/comcrt0.rel: src/util/com/comcrt0_z80.s
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDAS) -plosgff comcrt0.rel ../../../src/util/com/comcrt0_z80.s

build/util/z80/bdos.rel: src/util/com/bdos_z80.s
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDAS) -plosgff bdos.rel ../../../src/util/com/bdos_z80.s

build/util/z80/%.rel: src/util/%.c src/os/cpm.h src/util/util.h
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDCC) $(SDFLAGS) -DCOM_PROGRAM -I../../../src \
	    -c ../../../src/util/$*.c -o $*.rel

build/util/z80/%.ihx: build/util/z80/%.rel build/util/z80/util.rel \
                      build/util/z80/comcrt0.rel build/util/z80/bdos.rel
	cd build/util/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0x0100 \
	    --data-loc 0x9000 comcrt0.rel bdos.rel util.rel $*.rel -o $*.ihx

build/util/z80/ED.COM: build/util/z80/ed.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@
build/util/z80/DUMP.COM: build/util/z80/dump.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@
build/util/z80/LOAD.COM: build/util/z80/load.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@
build/util/z80/PIP.COM: build/util/z80/pip.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@
build/util/z80/STAT.COM: build/util/z80/stat.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@
build/util/z80/SUBMIT.COM: build/util/z80/submit.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@

build/util/z80/mac.ihx: build/util/z80/mac.rel build/util/z80/util.rel \
                       build/util/z80/comcrt0.rel build/util/z80/bdos.rel
	cd build/util/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0x0100 \
	    --data-loc 0x8000 comcrt0.rel bdos.rel util.rel mac.rel -o mac.ihx

build/util/z80/MAC.COM: build/util/z80/mac.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@

build/util/z80/basic.rel: src/basic-e/basic.c src/util/util.h src/os/cpm.h
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDCC) $(SDFLAGS) -DCOM_PROGRAM -I../../../src \
	    -c ../../../src/basic-e/basic.c -o basic.rel

build/util/z80/basic.ihx: build/util/z80/basic.rel build/util/z80/util.rel \
                         build/util/z80/comcrt0.rel build/util/z80/bdos.rel
	cd build/util/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0x0100 \
	    --data-loc 0x6B00 comcrt0.rel bdos.rel util.rel basic.rel -o basic.ihx

build/util/z80/BASICBAS.COM: build/util/z80/basic.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@

build/util/z80/basic-launch.rel: src/basic-e/launcher.c src/util/util.h src/os/cpm.h
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDCC) $(SDFLAGS) -DCOM_PROGRAM -I../../../src \
	    -c ../../../src/basic-e/launcher.c -o basic-launch.rel

build/util/z80/basic-start.rel: src/basic-e/launcher_z80.s
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDAS) -plosgff basic-start.rel ../../../src/basic-e/launcher_z80.s

build/util/z80/basic-launch.ihx: build/util/z80/basic-launch.rel build/util/z80/util.rel \
                                build/util/z80/basic-start.rel build/util/z80/bdos.rel
	cd build/util/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0x0100 \
	    --data-loc 0x9000 basic-start.rel bdos.rel util.rel basic-launch.rel -o basic-launch.ihx

build/util/z80/BASIC.COM: build/util/z80/basic-launch.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@

build/util/z80/basic-int.rel: src/basic-e/int.c src/util/util.h src/os/cpm.h
	@mkdir -p build/util/z80
	cd build/util/z80 && $(SDCC) $(SDFLAGS) -DCOM_PROGRAM -I../../../src \
	    -c ../../../src/basic-e/int.c -o basic-int.rel

build/util/z80/basic-int.ihx: build/util/z80/basic-int.rel build/util/z80/util.rel \
                             build/util/z80/comcrt0.rel build/util/z80/bdos.rel
	cd build/util/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0x0100 \
	    --data-loc 0x8300 comcrt0.rel bdos.rel util.rel basic-int.rel -o basic-int.ihx

build/util/z80/BASICINT.COM: build/util/z80/basic-int.ihx tools/ihx2com.py
	$(PYTHON) tools/ihx2com.py $< $@

utility-disk: build/disks/utilities-A.img

build/disks/utilities-A.img: $(UTIL_COM_FILES) src/basic-e/demo.bas tools/mkdisk.py
	@mkdir -p build/disks
	$(PYTHON) tools/mkdisk.py 0 $@ \
	    ED.COM=build/util/z80/ED.COM \
	    DUMP.COM=build/util/z80/DUMP.COM \
	    LOAD.COM=build/util/z80/LOAD.COM \
	    PIP.COM=build/util/z80/PIP.COM \
	    STAT.COM=build/util/z80/STAT.COM \
	    SUBMIT.COM=build/util/z80/SUBMIT.COM \
	    MAC.COM=build/util/z80/MAC.COM \
	    BASIC.COM=build/util/z80/BASIC.COM \
	    BASICBAS.COM=build/util/z80/BASICBAS.COM \
	    BASICINT.COM=build/util/z80/BASICINT.COM \
	    DEMO.BAS=src/basic-e/demo.bas

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
             -Isrc/os -Isrc/os/i386 -Isrc/os/8080
I386SRC    = machine bios bdos ccp 8080/i8080
I386OBJ    = build/i386/boot.o \
             $(addprefix build/i386/,$(addsuffix .o,$(notdir $(I386SRC)))) \
             build/i386/z80.o \
             build/i386/plat_i386.o build/i386/rt.o

i386: build/i386/kernel.elf

# --- bootable ISO (GRUB/El Torito, BIOS boot) for Proxmox/QEMU/VirtualBox --
# The CP/M disks are not on the ISO: attach build/disks/system.img (made by
# tools/mkimage.py) as a separate IDE/SATA drive, and boot the ISO as the CD.
GRUBMKRESCUE ?= grub-mkrescue

iso: build/cpm.iso

build/cpm.iso: build/i386/kernel.elf src/os/i386/grub.cfg
	@mkdir -p build/iso/boot/grub
	cp build/i386/kernel.elf build/iso/boot/kernel.elf
	cp src/os/i386/grub.cfg build/iso/boot/grub/grub.cfg
	$(GRUBMKRESCUE) -o $@ build/iso

build/i386/boot.o: src/os/i386/boot.s
	@mkdir -p build/i386
	$(I386AS) $< -o $@

I386DEPS = src/os/cpm.h src/os/plat.h src/os/geom.h src/os/machine.h src/os/8080/i8080.h

build/i386/%.o: src/os/%.c $(I386DEPS)
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/%.o: src/os/8080/%.c $(I386DEPS)
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/z80.o: src/os/z80/z80.c src/os/cpm.h src/os/z80/z80.h
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/plat_i386.o: src/os/i386/plat_i386.c src/os/i386/boot_hdr.h src/os/cpm.h src/os/plat.h
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/rt.o: src/os/i386/rt.c
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/kernel.elf: $(I386OBJ) src/os/i386/link.ld
	$(I386CC) -T src/os/i386/link.ld -o $@ -ffreestanding -nostdlib -no-pie -Wl,--build-id=none \
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

# The system disk image for the i386 build: drives A/B/C packed into the
# single IDE image the kernel reads (src/os/i386/boot_hdr.h). Rebuilt whenever
# the per-drive images change; needed alongside build/cpm.iso for hypervisors
# (e.g. Proxmox) that cannot use QEMU's -kernel flag directly.
fsimage: build/disks/system.img

build/disks/system.img: build/disks/A.img build/disks/B.img build/disks/C.img tools/mkimage.py
	$(PYTHON) tools/mkimage.py -o $@ \
	    -A build/disks/A.img -B build/disks/B.img -C build/disks/C.img

# The system disk is rebuilt on every run so that it picks up the images.
run-i386: build/i386/kernel.elf build/disks/system.img
	$(QEMU386) -nographic -nic none -m 16 -kernel build/i386/kernel.elf \
	    -drive file=build/disks/system.img,format=raw,if=ide \
	    -serial mon:stdio -serial file:build/disks/list.out \
	    -device isa-debug-exit,iobase=0xf4,iosize=0x04
