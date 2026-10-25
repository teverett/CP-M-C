# `src/os` source survey

`src/os/` contains the C implementation of the CP/M 2.2 Console Command
Processor (CCP), Basic Disk Operating System (BDOS), and portable BIOS, plus
the machine and platform layers used to run them on the hosted test machine,
native Z80, and bare-metal i386 targets. The CCP and BDOS follow the original
8080 assembly under `orig/os/` routine by routine; the original binaries can
also be run by the hosted harness for differential testing.

## Shared CP/M implementation

| File | Responsibility |
| --- | --- |
| `ccp.c` | CCP startup and command loop, console input, command parsing and FCB construction, intrinsic commands (`DIR`, `ERA`, `TYPE`, `SAVE`, `REN`, `USER`), submit-file handling, and transient `.COM` loading. |
| `bdos.c` | BDOS entry and function dispatch, console and line-editing services, disk selection, FCB/directory operations, sequential and random record I/O, allocation and checksum state, and error handling. `bdos_call()` dispatches functions 0 through 40. |
| `bios.c` | Portable implementation of the BIOS jump-table services. It builds the in-memory disk parameter headers/blocks and vectors, tracks disk/track/sector/DMA state, translates sectors, and delegates console and physical-sector operations to `plat.h`. |
| `cpm.h` | Shared memory map, byte/word memory accessors, BIOS interface, BDOS/CCP entry points, and target-specific non-local-jump definitions. |
| `geom.h` | Three disk parameter sets used by this machine: A is 8-inch IBM 3740 single density, B uses 2K blocks and 16-bit allocation pointers, and C uses 4K blocks and extent mask 3. Four drive slots exist, but D is not mounted. |
| `plat.h` | Contract between the portable BIOS and platform implementation: console/list/punch/reader I/O, drive detection, 128-byte sector transfer, and process/guest exit. |

The caller-visible CP/M structures remain at their conventional addresses:
the low-memory vectors, FCBs, command buffer, disk parameter data, allocation
and checksum vectors, and DMA buffer are represented in the emulated 64K
`mem[]` on hosted/i386, and at those addresses in native Z80 memory. `cpm.h`
selects the 56K CP/M layout for hosted/i386 (CCP `DC00h`, BDOS `E400h`, BIOS
`F200h`) and a separate native Z80 layout (CCP data `A000h`, BDOS `A800h`,
BIOS table `A880h`, disk data `F300h`).

## Hosted and emulated-machine support

| File | Responsibility |
| --- | --- |
| `host.c` | Command-line host: stdin/stdout console, disk image files, list output, disk fault and byte-injection options, instruction limit, BDOS tracing, and loading the original CCP/BDOS reference binaries. |
| `host.h` | Private interface used to launch the native Z80 image on the hosted Z80 emulator. |
| `machine.c` | Hosted/i386 64K machine. Initializes low-memory vectors and BIOS traps, boots and warm-boots the system, dispatches BIOS/BDOS calls, and runs transient programs on the 8080 core. In reference mode the original CCP and BDOS execute on that core against the same BIOS. |
| `machine.h` | Machine-run interface and reference-binary, tracing, and execution-limit state. |
| `8080/i8080.c`, `8080/i8080.h` | 8080 CPU state and single-instruction execution core used for transient `.COM` programs and the original reference binaries. |

The hosted executable accepts `-A`/`-B`/`-C` disk images (missing images are
created and formatted), `-i` console input, `-l` list output, and `-T` BDOS
tracing. `-r DIR` runs `ccp.bin` and `bdos.bin` from a directory as the
reference implementation; `-z FILE` runs a native Z80 image on the Z80 core.
`-e` injects a failing disk track, `-x` changes an image byte after a specified
number of reads, and `-m` sets an emulated-instruction limit. See `host.c` for
the complete option parsing and argument formats.

## Native Z80 target

| File | Responsibility |
| --- | --- |
| `z80/crt0.s` | SDCC startup, low-memory entry stubs, BDOS register/stack glue, CCP entry points, and warm-boot entry. |
| `z80/native.c` | Native boot and warm-boot initialization, BDOS entry bridge, and dispatch into the CCP. Transient programs execute directly at `0100h`. |
| `z80/plat_z80.c` | Implements the portable platform interface using Z80 I/O ports. |
| `z80/z80_ports.h` | Shared port numbers for console, list/punch/reader, drive selection, track/sector/DMA, disk command/status, and exit. |
| `z80/z80.c`, `z80/z80.h` | Z80 state and instruction-step core used by the hosted emulator to run the SDCC image. |
| `z80/host_z80.c` | Loads the native image into the host memory array and connects the Z80 emulator's port callbacks to the common hosted console and disk services. |

The native image is built with SDCC; the Makefile places compiled code at
`A900h` and data at `F800h`. `crt0.s` supplies the required startup and CP/M
entry glue. BIOS calls made directly by transient programs are only fully
implemented for the entries explicitly provided there; other entries use a
stub.

## Bare-metal i386 target

| File | Responsibility |
| --- | --- |
| `i386/boot.s` | Kernel entry and boot handoff. |
| `i386/boot_hdr.h` | On-disk header and layout constants for the system disk, drive slots, and optional console script. |
| `i386/grub.cfg` | GRUB menu entry for the kernel. |
| `i386/link.ld` | Linker layout, including the early multiboot header and PVH note. |
| `i386/plat_i386.c` | Bare-metal platform services: serial console/list/diagnostics, ATA PIO sector I/O, disk presence, and system-disk header/script handling. |
| `i386/rt.c` | Small freestanding implementations of memory routines the compiler may emit calls to. |

The i386 kernel reuses `machine.c`, `bios.c`, `bdos.c`, `ccp.c`, and the 8080
core. It runs under QEMU, reads a packed IDE system disk, and uses serial
ports for console and list output. The build uses general registers only
(no SSE/x87); `make check-i386` checks the resulting kernel for disallowed
instructions. `make fsimage` packs the A/B/C images into the system disk;
`make iso` wraps the kernel in a BIOS/El Torito ISO.

## Build and verification

The target rules are in `src/os/Makefile`, included by the top-level
`Makefile`:

| Target | Output / purpose |
| --- | --- |
| `make build/cpm` | Hosted test executable using the portable C implementation and 8080 emulator. |
| `make z80` | SDCC native Z80 image at `build/z80/cpm.bin`. |
| `make i386` | Freestanding i386 kernel at `build/i386/kernel.elf`. |
| `make test` | Top-level differential suite: runs equivalent console input and disk images against the original assembly reference and supported port targets, comparing output and resulting state. |
| `make run-i386` | Boots the i386 kernel in QEMU with the system disk. |
| `make fsimage`, `make iso` | Build the packed i386 disk image and bootable ISO, respectively. |

The original assembly used for reference binaries is in `orig/os/ccp.asm` and
`orig/os/bdos.asm`; `orig/os/Makefile` assembles them at the 56K addresses
(`DC00h` and `E400h`) with `tools/asm80.py`. These binaries run through the same
BIOS services as the C port, allowing tests to compare behavior without
requiring byte-identical C and assembly output.

## Porting constraints

- Preserve the shared CP/M memory layout and 8080-visible return conventions
  when changing the CCP, BDOS, or BIOS interfaces; transient programs can
  inspect these structures directly.
- Keep disk geometry and sector translation consistent between `geom.h`,
  the BIOS, and each platform's 128-byte transfer implementation.
- The hosted and i386 builds execute transient programs and reference binaries
  on the 8080 core, while the native build executes them on the Z80 CPU.
  Platform-specific startup, memory layout, and direct BIOS-call behavior
  therefore differ.
- The C port aims at behavioral equivalence, not byte-for-byte reproduction
  of the original machine code. The top-level README describes known
  differences and test coverage.
