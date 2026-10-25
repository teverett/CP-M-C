# `orig/os` assembly reference

`orig/os/` contains the CP/M 2.2 CCP and BDOS in reformatted 8080 assembly.
These are the original-code reference inputs for this repository's C port;
they are not the broader, mixed-version archive in `orig/cpm2-plm/`.

## Files

| File | Role |
| --- | --- |
| `ccp.asm` | Console Command Processor. It reads and parses command lines, handles the intrinsic commands, supports `$$$.SUB` command submission, selects drives/users, loads `.COM` files into the transient program area, and returns to the prompt when a transient program exits. The source documents the CCP entry points and command-buffer layout at its base. |
| `bdos.asm` | Basic Disk Operating System, version 2.2 with Digital Research Patch 01. It implements the BDOS entry and function dispatch, console services and line editing, disk/FCB and directory operations, sequential and random file access, allocation, and error handling through BIOS calls. |
| `Makefile` | Builds the reference CCP and BDOS binaries used by the hosted harness. |
| `README.md` | Upstream source background, formatting/assembler notes, and an assembly-fidelity statement. |
| `LICENSE.txt` | Permission statement for use, distribution, and modification of CP/M and derivatives; retain it with this source. |

## Source provenance and format

The source comments identify `ccp.asm` as reformatted from the `os2ccp.asm`
file in the CP/M 2.2 PL/M archive and `bdos.asm` as reformatted from
`os3bdos.asm`; the BDOS also incorporates Digital Research CP/M 2.2 Patch 01.
Eric Smith's 2018 reformatting split multiple instructions per line, adjusted
symbols and expressions for cross-assembly, and added an origin override.
The upstream README says the reformatted code was verified against binaries
from CP/M 2.2 disks, except for the six-byte CCP/BDOS serial numbers.

These files are assembly, not the PL/M versions in `orig/cpm2-plm/`. The CCP
source describes its base layout, including its start/clear entry points and
command buffer. The BDOS exposes the standard low-memory BDOS entry and error
handler words and calls BIOS services through the BIOS jump table. Treat
addresses, entry conventions, and data layout as part of the interface when
editing either file.

## Building the reference binaries

The top-level build includes `orig/os/Makefile`. It assembles the sources with
`tools/asm80.py`, passing an explicit origin and producing a binary plus a
symbol file:

| Output | Input | Origin |
| --- | --- | --- |
| `build/ref/ccp.bin` | `orig/os/ccp.asm` | `DC00h` |
| `build/ref/bdos.bin` | `orig/os/bdos.asm` | `E400h` |

Build both with `make build/ref/ccp.bin build/ref/bdos.bin`, or as part of
`make test`. The hosted machine loads these files with `build/cpm -r
build/ref`; the reference code then runs on the 8080 emulator while using the
same BIOS services as the C port.

## How the two modules fit together

- The CCP provides the interactive command layer and transient-program
  launcher. It invokes BDOS for file, directory, console, drive, and user
  operations.
- The BDOS implements the user-program file and console interface and uses
  BIOS entry points for physical console and disk operations.
- The BDOS source contains the disk-management machinery: it reads the
  selected drive's BIOS-provided disk parameter data, manages directory and
  allocation state, maps FCB records to disk blocks, and coordinates
  sequential/random reads and writes.
- The BIOS itself is not implemented in these two assembly files. In the
  reference test mode the emulator traps the BIOS jump-table entries and
  dispatches them to the portable BIOS in `src/os/bios.c`.

The source targets the 8080 instruction set. The C translation is intended
to preserve behavior, not object-code identity; the repository's differential
tests compare observable output and disk results between the original modules
and the port.
