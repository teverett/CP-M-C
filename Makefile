CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -O2 -g
PYTHON  ?= python3

SRC = src/host.c src/host_z80.c src/machine.c src/bios.c src/bdos.c \
      src/ccp.c src/i8080.c src/z80.c

all: build/cpm build/ref/ccp.bin build/ref/bdos.bin z80 i386

build/cpm: $(SRC) src/cpm.h src/i8080.h src/plat.h src/geom.h src/machine.h src/host.h src/z80.h src/z80_ports.h
	@mkdir -p build
	$(CC) $(CFLAGS) -DCPM_HOSTED -o $@ $(SRC)

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

.PHONY: all test clean z80 i386 run-i386

# --- native Z80 build (SDCC) -------------------------------------------
# The same C compiled to Z80 code.  Memory map: TPA 0100-9FFF, CCP data at
# A000, BDOS entry page A800, compiled code from A900.
SDCC    ?= sdcc
SDAS    ?= sdasz80
Z80SRC   = native plat_z80 bios bdos ccp
Z80OBJ   = $(addprefix build/z80/,crt0.rel $(addsuffix .rel,$(Z80SRC)))
SDFLAGS  = -mz80 --std-c99 --opt-code-size

z80: build/z80/cpm.bin

build/z80/crt0.rel: src/crt0.s
	@mkdir -p build/z80
	cd build/z80 && $(SDAS) -plosgff crt0.rel ../../src/crt0.s

build/z80/%.rel: src/%.c src/cpm.h src/plat.h src/geom.h src/z80_ports.h
	@mkdir -p build/z80
	cd build/z80 && $(SDCC) $(SDFLAGS) -c ../../src/$*.c -o $*.rel

build/z80/cpm.ihx: $(Z80OBJ)
	cd build/z80 && $(SDCC) -mz80 --no-std-crt0 --code-loc 0xA900 \
	    --data-loc 0xF800 $(notdir $(Z80OBJ)) -o cpm.ihx

build/z80/cpm.bin: build/z80/cpm.ihx
	$(PYTHON) tools/ihx2bin.py $< $@

# --- bare-metal i386 build (run under QEMU) -------------------------------
I386CC    ?= i686-elf-gcc
I386AS    ?= i686-elf-as
QEMU386   ?= qemu-system-i386
I386FLAGS  = -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra \
             -Isrc -Isrc/i386
I386SRC    = machine bios bdos ccp i8080
I386OBJ    = build/i386/boot.o $(addprefix build/i386/,$(addsuffix .o,$(I386SRC))) \
             build/i386/plat_i386.o build/i386/rt.o

i386: build/i386/kernel.elf

build/i386/boot.o: src/i386/boot.s
	@mkdir -p build/i386
	$(I386AS) $< -o $@

build/i386/%.o: src/%.c src/cpm.h src/plat.h src/geom.h src/machine.h src/i8080.h
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/plat_i386.o: src/i386/plat_i386.c src/i386/boot_hdr.h src/cpm.h src/plat.h
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/rt.o: src/i386/rt.c
	@mkdir -p build/i386
	$(I386CC) $(I386FLAGS) -c $< -o $@

build/i386/kernel.elf: $(I386OBJ) src/i386/link.ld
	$(I386CC) -T src/i386/link.ld -o $@ -ffreestanding -nostdlib -no-pie $(I386OBJ) -lgcc

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
