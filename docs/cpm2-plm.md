# `orig/cpm2-plm` source survey

`orig/cpm2-plm/` is a flat archive of CP/M assembly and PL/M sources, compiler
listings, notes, and prebuilt command files. It is separate from `orig/os/`,
which contains the CCP and BDOS assembly used as the reference for this
repository's C port. The files here cover several CP/M releases and tools;
they should not be treated as one uniform, directly buildable release.

## CP/M system sources

| File | Role |
| --- | --- |
| `OS1BOOT.ASM` | CP/M cold-start loader; establishes the system base and BDOS entry relationships. |
| `OS2CCP.ASM` | CP/M 2.2 Console Command Processor: command input/parsing, FCB setup, drive selection, transient command loading, and return to the CCP. |
| `OS3BDOS.ASM` | Primary CP/M 2.2 Basic Disk Operating System implementation. |
| `OS3BDOS1.ASM` | Alternate BDOS source copy. `README.TXT` describes a small code-sequence difference from `OS3BDOS.ASM`; it is a variant, not another subsystem. |
| `OS4BIOS.ASM` | CP/M 2.2 MDS-800 BIOS implementation for four single-density drives, including controller I/O and disk handling. |
| `OS5TRINT.SRC` | Transient-program interface definitions for fixed CP/M locations and entry points, including names used by PIP. |

The versions are mixed: for example, the CCP, BDOS, and MDS BIOS sources
identify themselves as CP/M 2.2, while `CBIOS.ASM`, `DEBLOCK.ASM`, and several
PL/M utilities describe CP/M 2.0 interfaces or assumptions.

## BIOS, disk, and system utilities

| File | Role |
| --- | --- |
| `BIOS.ASM` | Another MDS-800 CP/M 2.2 BIOS source, alongside `OS4BIOS.ASM`. |
| `CBIOS.ASM` | CP/M 2.0 customization-BIOS skeleton: defines the BIOS vector and memory layout but leaves machine-specific I/O to be supplied. |
| `CPMOVE.ASM` | Relocates a CP/M 2.2 system image and patches its BDOS entry arrangement; its closing instructions direct the operator to `SYSGEN`. |
| `SYSGEN.ASM` | MDS system-generation utility. Its disk constants describe tracks, sectors, skew, drives, and the system-image load point. |
| `DEBLOCK.ASM` | Reusable logical-record/physical-sector deblocking logic for CP/M 2.0 disk I/O. |
| `DISKDEF.LIB` | Assembler macro library for defining disk geometry and allocation parameters used by disk routines. |
| `DUMP.ASM` | Standalone file dumper that reads a file through BDOS and displays its contents in hexadecimal. |
| `ED20PAT.ASM` | Machine-code replacement for ED 2.0's `mem$move` routine; it relies on ED-specific internal addresses. |
| `README.PIP` | Brief PIP usage note explaining the `[R` option for copying read-only/System files. |
| `README.TXT` | Archive note describing the preserved alternate BDOS source. |

`BIOS.ASM` and `OS4BIOS.ASM` are parallel source files for the MDS BIOS; the
archive does not explain why both names are retained. Likewise, it does not
establish the exact relationship between `CPMOVE.ASM` and the `MOVCPM.COM`
images.

## Resident assembler

The assembler is split into modules with fixed origins and direct
cross-module references:

| File | Responsibility |
| --- | --- |
| `AS0COM.ASM` | Shared data, stack/end-memory setup, and storage parameters. |
| `AS1IO.ASM` | File and console I/O, buffering, diagnostics, and assembly termination. |
| `AS2SCAN.ASM` | Source scanning and numeric conversion. |
| `AS3SYM.ASM` | Symbol-table initialization, lookup, insertion, and value access. |
| `AS4SEAR.ASM` | Binary-table search, including opcode lookup support. |
| `AS5OPER.ASM` | Operand scanning and expression evaluation. |
| `AS6MAIN.ASM` | Main assembly pass, opcode/directive dispatch, and output generation. |

`AS6MAIN.ASM` documents the module origins and imports the other modules by
their fixed addresses. `AS3SYM.ASM` describes macro records, but notes that
macro handling is not implemented there.

## DDT debugger

| File | Responsibility |
| --- | --- |
| `DDT0MOV.ASM` | Relocation stub that moves the debugger module and adjusts its BDOS-relative destination. |
| `DDT1ASM.ASM` | DDT disassembly/assembly module, with conditional debug and relocation modes. |
| `DDT2MON.ASM` | Monitor: program loading, machine-state handling, breakpoints, and monitor commands. |

## PL/M programs and listings

| Source | Role | Companion listing |
| --- | --- | --- |
| `ED.PLM` | ED text editor, including command processing and macro support. | `ED.LIN` |
| `LOAD.PLM` | Reads Intel HEX input and writes a command file. | `LOAD.LIN` |
| `PIP.PLM` | Peripheral Interchange Program for file/device transfers; includes CP/M-version checks and patchable I/O areas. | `PIP.LIN` |
| `STAT.PLM` | STAT status and file-information utility. | `STAT.LIN` |
| `SUBMIT.PLM` | Builds a command file from submitted commands and reboots to execute it. | `SUBMIT.LIN` |

The `.LIN` files are compiler/link listings, not source modules or build
scripts. `OS5TRINT.SRC` supplies interface names expected by transient
programs. `PIP22.CMD` is an opaque command-file artifact; the included notes
do not fully specify how it relates to the PIP source or executable.

## XSUB

| File | Role |
| --- | --- |
| `XSUB0.ASM` | Relocation/setup code; checks the CP/M version and whether XSUB is already present. |
| `XSUB1.ASM` | XSUB 2.2 implementation, loaded below the CCP to provide command input to programs. |

## Prebuilt artifacts

The directory also includes `.COM` binaries for `ASM`, `DDT`, `DUMP`, `ED`,
`ED1`, `LIB`, `LINK`, `LOAD`, `MAC`, `MOVCPM`, `MOVCPM1`, `PIP`, `STAT`,
`SUBMIT`, `SYSGEN`, `SYSGEN1`, and `XSUB`, as well as `PIP22.CMD`.
`MAC-IMAGE.ASM` is a byte-preserving 8080 control-flow disassembly of
`MAC.COM`. It renders reachable instructions as mnemonics and retains
unreachable bytes as `DB` data, with printable ASCII runs rendered as
assembler string literals; indirect control flow can leave executable bytes
undecoded. Regenerate it with
`python3 tools/dis8080.py orig/cpm2-plm/MAC.COM -o orig/cpm2-plm/MAC-IMAGE.ASM`
and assemble it with `python3 tools/asm80.py -o build/MAC.COM
orig/cpm2-plm/MAC-IMAGE.ASM`. The generated image reproduces the binary
exactly; this is not the original MAC source. There is no source here for
several other tools, including `LIB` and `LINK`. The archive does not
document the variant differences between
`ED.COM`/`ED1.COM`, `MOVCPM.COM`/`MOVCPM1.COM`, or
`SYSGEN.COM`/`SYSGEN1.COM`; do not infer them from the filenames alone.

## Build and maintenance notes

- The resident assembler's fixed module origins are part of its source-level
  linkage; changing an origin requires checking the corresponding imports in
  the other modules.
- `DEBLOCK.ASM` depends on disk-parameter definitions, supplied through
  `DISKDEF.LIB` or equivalent tables.
- `CPMOVE.ASM` and `SYSGEN.ASM` are related in the system preparation workflow,
  but the source does not provide a complete reproducible build sequence.
- The presence of `.LIN` files does not supply the PL/M compiler commands or
  linker options. No build recipe for this archive is included.
- Preserve the paired and alternate sources as separate historical artifacts
  unless their provenance and intended replacement are independently
  established.
