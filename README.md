[![build](https://github.com/teverett/CP-M-C/actions/workflows/build.yml/badge.svg)](https://github.com/teverett/CP-M-C/actions/workflows/build.yml)

# CP/M 2.2 CCP and BDOS in C

A routine-by-routine translation of the Digital Research CP/M 2.2 console
command processor (CCP) and basic disk operating system (BDOS) from 8080
assembly to C.  The assembly source is in `orig/os/` (Eric Smith's reformatted
version, <https://github.com/brouhaha/cpm22>, includes DRI patch 01).

## What "converted" means here

* `src/os/bdos.c` and `src/os/ccp.c` follow `orig/os/bdos.asm` and `orig/os/ccp.asm`
  one routine at a time: same names, same variables, same control flow
  (`goto`s included).  Conditions the assembly returned in the Z/C flags are
  C return values; 8080 register pairs are `uint16_t`; 8-bit arithmetic is
  done in `uint8_t` so wrap-around matches.
* Everything that other code can see lives in the 64K CP/M address space (a
  `mem[]` array in the hosted and i386 builds, the CPU's own memory in the
  native Z80 build): FCBs, the command buffer, disk parameter headers/blocks, allocation and checksum
  vectors, the DMA buffer.  The BIOS is the `cpm_bios` struct in `src/os/cpm.h`
  (one member per BIOS jump-table entry).
* It is **not** byte-identical code: a C compiler cannot reproduce the
  1980 hand-written 8080 object code.  Equivalence is established by
  differential testing instead (below).  The CCP's serial-number check is
  kept; the original's `DI`/`HLT` trap becomes `cpm_halt()`.

## Quick start

    make test                      # build everything and run the differential tests
    build/cpm -A a.img -i script   # run the C port on the hosted test machine
    make run-i386                  # boot CP/M on a bare-metal i386 kernel in QEMU
    ./run.sh                       # same, as a standalone script (builds first if needed)
    make iso                       # build a bootable ISO (BIOS/El Torito) -> build/cpm.iso
    make fsimage                   # build the i386 system disk -> build/disks/system.img

Prerequisites: a C compiler, `make` and Python 3 for the hosted build and the
tests; [SDCC](https://sdcc.sourceforge.net/) for the Z80 build; an i386 cross
compiler (`i686-elf-gcc`, or Ubuntu's `gcc-i686-linux-gnu`) and
`qemu-system-i386` for the i386 build.  On macOS: `brew install sdcc
i686-elf-gcc qemu`.  `make` builds everything; build a single target with
`make build/cpm`, `make z80` or `make i386`.

## Layout

| path | what |
| --- | --- |
| `src/os/cpm.h` | memory map per target, BIOS interface, entry points |
| `src/os/bdos.c`, `src/os/ccp.c` | the port (identical source for every target) |
| `src/os/bios.c`, `src/os/plat.h`, `src/os/geom.h` | portable BIOS: disk tables, translation; platform interface |
| `src/os/machine.c` | hosted/i386: 64K memory, boot, runs `.COM` programs |
| `src/os/8080/` | the 8080 CPU core (runs `.COM` programs and the original binaries) |
| `src/os/host.c` | test host: stdio console, disk image files, options |
| `src/os/z80/` | Z80: the native build (SDCC: startup, BDOS entry, port I/O), plus the Z80 core and virtual Z80 machine used to test it |
| `src/os/i386/` | bare-metal i386 build: multiboot stub, UART/ATA/QEMU platform |
| `orig/os/` | original assembly |
| `tools/` | `asm80.py` (8080 assembler for `orig/os/`), `mkdisk.py`, `mkimage.py`, `ihx2bin.py` |
| `tests/` | differential test suite and test programs |

## Targets

The same `bdos.c`/`ccp.c`/`bios.c` build three ways:

    make              # everything below
    make z80          # native Z80 code with SDCC   -> build/z80/cpm.bin
    make i386         # bare-metal i386, for QEMU   -> build/i386/kernel.elf
    make check-i386   # verify the kernel uses general registers only
    make run-i386     # boot it in QEMU on the serial console (Ctrl-A X quits)
    make test         # differential tests on every target

* **hosted** (`build/cpm`): the C is compiled for your machine and drives a
  64K `mem[]`.  `.COM` programs run on the built-in 8080 core.  This is the
  test machine: console on stdin/stdout, disk images as files.
* **z80** (`make z80`): compiled by SDCC to Z80 code.  The CPU's address space
  *is* the CP/M address space; programs run natively.  Memory map: TPA
  `0100`-`9FFF`, CCP data `A000`, BDOS entry page `A800`, compiled code from
  `A900` (about 10 KB), the BIOS tables at `F300`.  Console and disks are I/O
  ports (`src/os/z80/z80_ports.h`); `build/cpm -z build/z80/cpm.bin` runs the image
  on the Z80 core in the test host.  SDCC has no 8080 backend, so this is Z80
  code, not strict 8080 (`.COM` programs, being 8080 code, run fine on it).
* **i386** (`make i386`): a freestanding 32-bit kernel (`i686-elf-gcc`, or
  override `I386CC`/`I386AS`) that QEMU boots with `-kernel`, through either
  the multiboot header or the PVH ELF note.  Console is COM1, the list device
  COM2, diagnostics COM3, and the disks live on an IDE disk
  (`tools/mkimage.py` builds the system disk; layout in
  `src/os/i386/boot_hdr.h`).  `.COM` programs run on the same 8080 core as in the
  hosted build.  The kernel has no interrupt table and never enables SSE, so
  it is compiled with `-mgeneral-regs-only` (`make check-i386` enforces it).
  `make run-i386` creates blank formatted disks in `build/disks/` if needed;
  put images built with `tools/mkdisk.py` there to use your own.
* **fsimage** (`make fsimage`): builds the i386 system disk,
  `build/disks/system.img`, from blank formatted drives in `build/disks/`
  (`A.img`/`B.img`/`C.img`, created by `tools/mkdisk.py` if missing — drop
  your own images there first to use them) packed by `tools/mkimage.py`.
  `run-i386` depends on this target too, so `make fsimage` on its own is
  mainly for pairing with `make iso` when QEMU itself isn't driving the VM
  (e.g. handing both files to Proxmox).
* **iso** (`make iso`): wraps `build/i386/kernel.elf` in a GRUB/El Torito
  bootable ISO (`build/cpm.iso`) for hypervisors that boot from CD rather than
  QEMU's `-kernel` flag, such as Proxmox. Needs `grub-mkrescue` (Homebrew:
  `brew install i686-elf-grub`, then pass `GRUBMKRESCUE=i686-elf-grub-mkrescue`;
  Debian/Ubuntu: `apt install grub-pc-bin xorriso`). GRUB loads the kernel via
  the multiboot header, same as QEMU's `-kernel`. The CP/M disks are *not* on
  the ISO: in Proxmox, create the VM as BIOS/SeaBIOS (not UEFI), attach
  `build/cpm.iso` as the CD-ROM (boot order: CD first), and attach
  `build/disks/system.img` (`make fsimage`, below) as an IDE or SATA drive —
  console is the VM's serial port (COM1), not the display. Sanity-check
  locally first:

      qemu-system-i386 -m 16 -cdrom build/cpm.iso -boot d \
          -drive file=build/disks/system.img,format=raw,if=ide -serial mon:stdio

## Hosted test machine

    build/cpm -A a.img -B b.img -i script.txt

`-A`..`-C` give disk images (created and formatted if missing, written back
on exit).  Console input comes from `-i FILE` or stdin; the program exits
when the input runs out.  Drive A is an 8" IBM 3740 disk, B has 2K blocks
with 16-bit block pointers, C has 4K blocks (extent mask 3); D is not
mounted.  Other options: `-r DIR` (run the original binaries), `-z FILE`
(run a native Z80 image), `-t` (type-ahead visible to the BDOS), `-e TRK`
(fail disk I/O on a track), `-x N:D:OFF:V` (change an image byte mid-run),
`-T` (trace BDOS calls).

## How the port is tested

`build/cpm -r build/ref ...` runs the **original** CCP and BDOS, assembled
from `orig/os/` by `tools/asm80.py` at the standard 56K addresses
(CCP `DC00`, BDOS `E400`, BIOS `F200`), on the 8080 core, using the very same
BIOS routines.  `tests/run_tests.py` feeds identical console input and
identical starting disk images to the original and to each target (hosted C,
native Z80 on the Z80 core, bare-metal i386 under QEMU) and requires identical
console output, list output, exit status and final disk images.

Run it with `make test` or `python3 tests/run_tests.py` (`-k NAME` selects
cases, `--targets port,z80,i386` selects targets, `--fuzz-seeds N` runs more
random call sequences, `-v` lists passing cases too).

The suite (165 cases) covers CCP command parsing and every intrinsic, line
editing, submit files, error paths (bad sector, select error, R/O disk, R/O
file, disk and directory full), console functions, extents and modules, the
three disk geometries, and seeded random BDOS call sequences.  On the port,
99.4% of BDOS lines and 99.1% of CCP lines execute.  Not exercised: paths
that need an 8 MB file or a failing close of the submit file, and a few that
look unreachable.

Known differences from running the original on real hardware: in the hosted
and i386 builds the CCP and BDOS code is not in `mem[]`, so a program that
reads or patches it sees nothing there, and a stray jump into it does not
execute it (jumps to the CCP entry points `CCP_BASE` and `CCP_BASE+3` are
honoured).  In the native Z80 build the memory map differs (above), and BIOS
jump-table entries other than warm boot are stubs for programs that call the
BIOS directly.  Register contents other than the documented BDOS results (A,
B, HL) are not reproduced.

## Continuous integration

`.github/workflows/build.yml` has two jobs.  On Ubuntu it builds the Z80 image
and the i386 kernel (SDCC and `gcc-i686-linux-gnu` from apt), boots the kernel
once as a smoke test, runs the whole differential suite plus extra fuzz seeds,
and uploads the binaries as an artifact.  On macOS (Homebrew `sdcc`,
`i686-elf-gcc`, `qemu`) it does a plain `make`, checks the kernel, and runs
the same suite.  Dependabot keeps the actions up to date
(`.github/dependabot.yml`).

## License

The C port is BSD 3-Clause licensed; see `LICENSE`. The original assembly
source is Copyright Digital Research; see `orig/os/LICENSE.txt`.
