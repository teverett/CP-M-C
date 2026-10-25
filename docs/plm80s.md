# `orig/plm80s` source survey

`orig/plm80s/` is an archive of Intel's PL/M-80 version 2.0 cross-compiler
for the 8080. The compiler is written in FORTRAN IV and is split into two
passes. The directory also includes a short PL/M language guide, compiler
control documentation, and a sample program with generated output. This is a
compiler-source archive, distinct from the CP/M CCP/BDOS sources in
`orig/os/` and the historical system/tool sources in `orig/cpm2-plm/`.

## Files

| File | Role |
| --- | --- |
| `PLM81.FOR` | Pass 1 of the compiler. Reads PL/M source, scans and parses it, checks declarations and statement structure, manages symbols and literals, and writes intermediate compiler files for pass 2. Its comments document syntax errors, table limits, host assumptions, and installation-sensitive file units. |
| `PLM82.FOR` | Pass 2. Reads the intermediate files, performs storage and register allocation, generates 8080 machine code, and can emit object output and compiler listings. Its comments document code-generation errors, memory limits, and host configuration. |
| `PLMLANG.DOC` | Capsule summary of PL/M-80 syntax, types, expressions, statements, declarations, procedures, and built-ins. It explicitly is not a complete language specification. |
| `PLMCOMP.DOC` | Describes the two-pass compilation flow and compiler controls, including input/listing options, code origin, symbol table, and object format selection. |
| `PLMSAMP.PLM` | Example PL/M program. It computes square roots for integers 1 through 1000 and demonstrates procedures, declarations, loops, based variables, built-ins, and I/O through the Intel monitor. |
| `PLMSAMP.PRN` | Sample compiler listing, including source with line/block information, generated 8080 mnemonics, and compilation status. |
| `PLMSAMP.HEX` | Sample compiler output: a symbol/address listing followed by Intel HEX object records. |
| `README.PLM` | Archive provenance, contents, historical installation notes, and caveats about available documentation and compiler portability. |

The documentation files use a legacy text format; some tools identify them as
generic data rather than ordinary text. `README.PLM` describes the sources as
extracted from distribution tapes for the University of Michigan's MTS
mainframe system, and reports successful installations on MTS and VM/CMS.

## Compiler structure and data flow

1. **Pass 1 (`PLM81.FOR`)** reads PL/M source records, recognizes language
   constructs, checks syntax and declarations, tracks block/symbol
   information, and writes intermediate code and symbol information. Source
   echo/listing and syntax diagnostics are configurable.
2. **Pass 2 (`PLM82.FOR`)** consumes those intermediate files, resolves
   storage and labels, allocates registers, generates code for the MCS-80
   (8080), and can produce a symbol table, address/source cross-reference,
   decoded mnemonic listing, and object output.
3. **Object output** may be Intel HEX or BNPF according to pass-2 controls.
   The sample `.HEX` file demonstrates the HEX output form; the archive does
   not include a command script that runs both passes.

The compiler's implementation comments describe the intermediate files as
host Fortran files and note that their logical units, record handling, and
character translation may need adjustment for a particular host. Therefore,
these sources are not a drop-in build for the repository's CP/M make targets.

## PL/M-80 language notes

`PLMLANG.DOC` presents PL/M as a small, block-structured language for 8080
systems, based on PL/I. The summary describes `BYTE` and `ADDRESS` values,
arrays, `BASED` variables, `DATA` and `INITIAL` declarations, `LITERALLY`
text substitution, procedures, labels, interrupt procedures, and structured
`DO`, `IF`, `CASE`, and `GO TO` statements. It also lists arithmetic,
Boolean, relational, address (`.`), and built-in operations. This file is
introductory reference material rather than a complete or authoritative
language specification.

## Compiler controls and constraints

Compiler-control records begin with `$` and can change options during pass 1
or at the start of pass 2. `PLMCOMP.DOC` summarizes controls for source
margins and echo, output width, generated-code origin, symbol and
cross-reference listings, memory initialization display, and HEX versus
BNPF object output. Consult the archived control document for defaults and
the exact control letters; they vary by pass.

The FORTRAN source comments call out several porting constraints:

- Both passes assume host integer words with at least 31 usable bits.
- FORTRAN logical units for source, intermediate files, listings, and object
  output may need adaptation to the host's runtime and file conventions.
- The PL/M character-set translation table may need changing if the host
  character set differs.
- Pass 1's symbol and literal tables and pass 2's program-memory and symbol
  tables are fixed-size arrays; the comments explain which declarations and
  initialization data must change together to resize them.
- Pass 2 models a 16-bit 8080 address space and reports errors when program
  and variable storage exceed its supported memory.

`README.PLM` says the code was intended to be portable across FORTRAN IV
systems and notes successful use with several mainframe FORTRAN compilers.
It also warns that running it on an 8080/Z80 microcomputer may be difficult
because of the size of the two compiler modules. The archive contains no
modern build system, automated tests, or installation procedure, so successful
compilation on a new host should not be assumed.

## Provenance and preservation

The README identifies Intel copyright in the compiler source and describes
the archive as historically made available, while also acknowledging that the
distribution history and terms have become unclear. Preserve the original
notices and consult the included material and project licensing guidance
before redistributing or adapting these files; the README's historical
description should not be treated as independent legal advice.
