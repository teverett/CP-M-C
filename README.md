# CP/M 2.2 CCP and BDOS in C

A routine-by-routine translation of the Digital Research CP/M 2.2 console
command processor (CCP) and basic disk operating system (BDOS) from 8080
assembly to C.  The assembly source is in `orig/` (Eric Smith's reformatted
version, <https://github.com/brouhaha/cpm22>, includes DRI patch 01).

## What "converted" means here

* `src/bdos.c` and `src/ccp.c` follow `orig/bdos.asm` and `orig/ccp.asm`
  one routine at a time: same names, same variables, same control flow
  (`goto`s included).  Conditions the assembly returned in the Z/C flags are
  C return values; 8080 register pairs are `uint16_t`; 8-bit arithmetic is
  done in `uint8_t` so wrap-around matches.
* Everything that other code can see lives in a 64K array `mem[]`: FCBs, the
  command buffer, disk parameter headers/blocks, allocation and checksum
  vectors, the DMA buffer.  The BIOS is the `cpm_bios` struct in `src/cpm.h`
  (one member per BIOS jump-table entry).
* It is **not** byte-identical code: a C compiler cannot reproduce the
  1980 hand-written 8080 object code.  Equivalence is established by
  differential testing instead (below).  The CCP's serial-number check is
  kept; the original's `DI`/`HLT` trap becomes `host_halt()`.

## Layout

| path | what |
| --- | --- |
| `src/cpm.h` | memory map, BIOS interface, entry points |
| `src/bdos.c`, `src/ccp.c` | the port |
| `src/host.c` | demo "machine": console on stdin/stdout, disk images, BIOS |
| `src/i8080.c` | 8080 core, used to run `.COM` programs (and the original binaries for testing) |
| `orig/` | original assembly |
| `tools/asm80.py` | minimal 8080 assembler, enough for `orig/*.asm` |
| `tools/mkdisk.py` | builds test disk images |
| `tests/` | differential test suite and test programs |

## Build and run

    make            # builds build/cpm and the reference binaries
    make test       # differential test suite

    build/cpm -A a.img -B b.img -i script.txt

`-A`..`-C` give disk images (created and formatted if missing, written back
on exit).  Console input comes from `-i FILE` or stdin; the program exits
when the input runs out.  Drive A is an 8" IBM 3740 disk, B has 2K blocks
with 16-bit block pointers, C has 4K blocks (extent mask 3); D is not
mounted.  Transient `.COM` programs run on the built-in 8080 core.
Other options: `-t` (type-ahead visible to the BDOS), `-e TRK` (fail disk
I/O on a track), `-x N:D:OFF:V` (change an image byte mid-run), `-T` (trace
BDOS calls).

## How the port is tested

`build/cpm -r build/ref ...` runs the **original** CCP and BDOS, assembled
from `orig/` by `tools/asm80.py` at the standard 56K addresses
(CCP `DC00`, BDOS `E400`, BIOS `F200`), on the 8080 core, using the very same
BIOS routines.  `tests/run_tests.py` feeds identical console input and
identical starting disk images to both modes and requires identical console
output, list output, exit status and final disk images.

The suite (165 cases) covers CCP command parsing and every intrinsic, line
editing, submit files, error paths (bad sector, select error, R/O disk, R/O
file, disk and directory full), console functions, extents and modules, the
three disk geometries, and seeded random BDOS call sequences.  On the port,
99.4% of BDOS lines and 99.1% of CCP lines execute.  Not exercised: paths
that need an 8 MB file or a failing close of the submit file, and a few that
look unreachable.

Known differences from running the original on real hardware: the CCP and
BDOS code is not in `mem[]`, so a program that reads or patches it sees
nothing there, and a stray jump into it does not execute it (jumps to the CCP
entry points `CCP_BASE` and `CCP_BASE+3` are honoured).  Register contents
other than the documented BDOS results (A, B, HL) are not reproduced.

The original source is Copyright Digital Research; see `orig/LICENSE.txt`.
