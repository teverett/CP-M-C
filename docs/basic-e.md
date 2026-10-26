# `orig/basic-e` source survey

## C adaptation in this repository

`BASIC.COM` is a standalone Z80 launcher with two runtime overlays:
`BASICBAS.COM` executes `.BAS` source using `src/basic-e/basic.c`, and
`BASICINT.COM` loads original intermediate files using `src/basic-e/int.c`.
`BASIC filename` defaults to `.BAS`; use `BASIC filename.INT` explicitly
for compiled programs. The source path remains a bounded direct-source
subset and does not generate intermediate files.

`make utility-disk` installs the launcher, both overlays, and `DEMO.BAS`
alongside the other utilities; `./run.sh` boots that image. Run `BASIC DEMO`
at the CP/M prompt. Keep the overlays with the launcher: it searches the
current drive, then A:, for the selected runtime. The source file may
specify a different drive. The overlays replace the launcher in the TPA
rather than keeping both interpreters resident at once.

The adaptation supports floating-point expressions, numeric/string
variables, arrays, PRINT/INPUT, inline IF/THEN/ELSE, FOR/NEXT,
GOTO/GOSUB/RETURN, ON branches, DATA/READ/RESTORE, and common numeric
and string functions. Numeric labels are optional and source order is
preserved. Colons separate statements and backslashes continue lines.
As in the BASIC-E manual, FOR executes its body at least once and
reevaluates its TO/STEP expressions; STEP zero is permitted.

Limits are explicit: 6,142 source bytes, 256 indexed statements, 32
identifiers of up to 23 characters, 192 scalar/array cells, three array
dimensions, 512 bytes each of string and expression scratch storage,
255 bytes per string, 16 FOR/GOSUB levels, and eight expression levels.
Arrays require DIM and have inclusive, zero-based bounds. Numeric values
use IEEE single precision rather than the original floating-point format,
print with six significant digits, and Boolean results are zero or one.
INPUT accepts comma-separated fields but reports invalid input instead of
reprompting. Strings cannot contain NUL.

File I/O, DEF functions, FRE, hyperbolic functions, hardware I/O, and
compiler directives are not implemented in the direct-source path and
produce diagnostics when used. This is not a complete language-compatible
source compiler port. `tests/test_basic.py`
checks execution, diagnostics, recovery, and the linked memory layout on
hosted, native Z80, and i386 targets. BASIC uses the same Z80/BDOS bridge
as the other C-built utilities on hosted/i386.

`tests/test_basic_int.py` obtains compatibility fixtures by executing the
preserved original compiler as `BECOMP.COM`, then runs its actual `.INT`
output through the C runtime on all three targets. The original compiler
requires CRLF source records. These tests do not substitute a newly invented
bytecode format for the original intermediate representation.

The INT loader reads the original textual constants, size header, DATA
records, inline strings, and relocation operands. The bounded VM supports
arithmetic (including `^` exponentiation), 32-bit logical operations,
strings, arrays, branches, subroutines, DEF functions, and sequential file
I/O. Its separate limits are 2,048 code bytes, 256 DATA bytes, 64 constants,
64 scalar/array cells, four array dimensions, 384 string-storage bytes,
32 expression-stack slots, 16 subroutine levels, and two open files.
Exceeded limits and unsupported operations produce explicit diagnostics.

This is not a bit-exact replacement for the original monitor: constants
are evaluated with IEEE single precision, comparisons return zero or one,
and numeric formatting and string-storage management differ. Random-access
file mode, file EOF branches, and hardware input ports are not supported.
The direct-source subset and INT VM have different limits and feature sets;
running a `.BAS` file does not compile it to the original format.

## Original archive

`orig/basic-e/` preserves the BASIC-E compiler and run-time system for
8080 CP/M. BASIC-E was written by Gordon Eubanks, Jr. at the Naval
Postgraduate School; the compiler includes modifications for resident
operation credited to Gary Kildall. The archive includes PL/M sources,
8080 assembly support routines, two executable command files, and a
reference manual.

BASIC-E is a compiler plus a bytecode interpreter, not an interactive
line-editing BASIC interpreter and not a native 8080 code generator:

```text
program.BAS
    |
    v
BASIC.COM: scanner + parser + code synthesis
    |
    v
program.INT: constants + size information + intermediate instructions
    |
    v
RUN.COM: machine builder + interpreter + floating-point support
    |
    v
CP/M console and disk services
```

These are preserved historical sources, separate from the C OS and
standalone utilities in `src/`. The repository does not provide a build
recipe for this directory, and this survey does not establish that the
included binaries were built from these exact source revisions.

## File inventory

| File | Responsibility |
| --- | --- |
| `BASIC.PLM` | Compiler globals, CP/M I/O, source buffering/listing, scanner, token definitions, diagnostics, and intermediate-file output. |
| `BASCOM.LIT` | Shared compiler literals and limits, included by the three compiler modules. |
| `BASPAR.PLM` | Table-driven parser and compiler control loop, including both compilation passes. |
| `BASSYN.PLM` | Symbol table, label resolution, type checking, storage accounting, and intermediate-code generation for grammar productions. |
| `BUILD.PLM` | Run-time loader: reads an `.INT` file, converts constants, constructs machine areas, and relocates operands. |
| `RUN.PLM` | BASIC-E virtual-machine interpreter, dynamic storage, arrays/strings, console formatting/input, and file operations. |
| `FPINT.SRC` | 8080 interface between PL/M, CP/M, floating-point routines, memory moves, port I/O, and random-number support. |
| `FPPKG.SRC` | Core floating-point arithmetic, sign/absolute-value operations, normalization, comparison, and overflow support. |
| `FPCONV.SRC` | Numeric input/output and fixed/floating-point conversions. |
| `FPDATA.SRC` | Shared floating-point state, temporary storage aliases, constants, and random seed. |
| `TRAN.SRC` | Transcendental functions, square root, inverse division, and random-number generation. |
| `BASIC.COM` | Preserved compiler executable, not source. |
| `RUN.COM` | Preserved run-time executable, not source. |
| `manual.txt` | Language reference, operating instructions, grammar, compiler/run-time diagnostics, options, examples, and appended introductory notes. |

The revisions are mixed. `BASIC.PLM` retains a December 1976 CP/M 1.4
heading and notes for resident versions 1.4/1.5, while `BASPAR.PLM` prints
compiler version 2.1. `RUN.PLM` identifies a May 1977 CP/M version 2.0,
`BUILD.PLM` retains a version 1.3 heading but prints interpreter version 2.2,
and the manual is dated December 15, 1976. These labels describe the
historical components; they should not be read as one consistent release
number or a requirement for CP/M 2.2.

## Compiler organization

### Scanner and system I/O (`BASIC.PLM`)

The compiler obtains the source name from CP/M's default FCB at `005CH`,
forces its extension to `.BAS`, and reads 128-byte records. Its output
FCB uses `.INT`. The source is read twice, reopening it for the second
pass. `EMIT` buffers generated bytes and writes full intermediate-file
records through BDOS.

The scanner recognizes keywords, identifiers, numeric and string constants,
operators, statement separators, and continuation lines. It maintains the
current line and token text for the parser and error listing. Lowercase
conversion is optional; quoted string contents are preserved.

The resident modifications use CP/M BDOS at `0005H` and return through
`0000H`. `MON1` and `MON2` are placeholder procedures whose comments require
patching with jumps to BDOS. The initial `JMPTOMAIN` jump also requires
patching. This matters when reconstructing the compiler executable.

### Parser (`BASPAR.PLM`)

The parser contains read, lookahead, push, and apply-production tables,
with 341 states and 152 grammar productions. It saves token metadata on
parallel stacks and calls `SYNTHESIZE` when a production is reduced.
Initialization coordinates the scanner, symbol table, and synthesis module.

The first pass establishes symbols, labels, constants, and storage sizes.
The second generates the executable instruction stream. Error recovery
reinitializes statement parsing rather than discarding the entire compiler
session. End-of-program synthesis writes the size information between passes,
then emits the exit instruction, the intermediate-file end marker, and the
error count when the second pass completes.

### Symbols and synthesis (`BASSYN.PLM`)

Symbols are hashed through collision chains. Entries contain a print name,
type, address, and subtype; a flag records whether an address has been
resolved. The symbol table grows upward from available compiler storage,
while FOR/NEXT bookkeeping grows downward from the top of usable memory.

`SYNTHESIZE` implements semantic actions corresponding to the numbered
grammar productions. It handles numeric/string type checks, assignments,
expressions, arrays, user-defined functions, conditional branches,
FOR/NEXT, GOTO/GOSUB and computed branches, file declarations, input/output,
and program termination. Generation helpers distinguish literals, constants,
data statements, inline strings, and relocatable code references.

`BASCOM.LIT` centralizes limits including 32-byte identifier storage
(length plus name), 32 parser-stack entries, a 100-byte symbol-character
stack, 64 hash buckets, and at most 15 ON-branch targets. Source and
intermediate records are 128 bytes, text EOF padding is `1AH`, and the
continuation character is backslash.

## Intermediate file and run-time construction (`BUILD.PLM`)

The `.INT` file is a BASIC-E machine image description, not a CP/M `.COM`
program. The builder:

1. Opens the requested `.INT` file and initializes floating-point support.
2. Reads textual numeric constants delimited by `$`, ending that section
   at `*`, and converts each to a four-byte internal value.
3. Reads code/data sizes and the program-reference-table entry count,
   calculating the final machine layout and checking available memory.
4. Loads instructions, data, and inline strings, relocating branch,
   procedure, EOF-handler, and constant operands.
5. Stops at `7FH` and transfers control through the interpreter entry.

Two-byte size/address parameters are read high byte first and converted
to 8080 address order where needed. `DAT` directs bytes into the DATA area;
`ILS` becomes an inline string with a length byte. `BRS`, `BRC`, `PRO`,
`DEF`, and `CON` need operand adjustment rather than simple byte copying.

The four shared parameters `PARAM1`–`PARAM4` convey the code area, data area,
program reference table (PRT), and stack locations to the interpreter.
The builder is temporary: interpreter initialization moves the loaded image
down, making the builder's space available for subsequent dynamic allocation.

## Interpreter (`RUN.PLM`)

The interpreter executes a zero-address stack machine with working
registers A/B and a code pointer. Its dispatch covers operations numbered
0–96, with additional compact literal handling. Four-byte values hold
numbers or the machine's address/string representations.

The principal memory areas are the numeric constants, code, DATA statements,
PRT, evaluation stack, and free storage area (FSA). Strings and arrays
are dynamically allocated. `AVAILABLE`, `GETSPACE`, and `RELEASE` manage
free storage; string assignment/freeing tracks shared references, and
array-addressing routines calculate multidimensional element positions.
The evaluation stack wraps rather than providing conventional overflow
protection, a limitation also noted in the manual.

The run-time implements arithmetic and comparisons, numeric/string
conversions, concatenation/substrings, branches/subroutines, array access,
DATA/READ/RESTORE, formatted PRINT, prompted INPUT, and file I/O. The source
allows 20 user files and uses 128-byte CP/M disk records. BASIC-E files
contain ASCII fields; unblocked files are sequential, while declared
fixed-length logical records support sequential or random access.
Per-file EOF branches implement `IF END #... THEN ...`.

Diagnostics use short error codes and can include the current source line
when the compiler emits line-number instructions. Console input timing
updates a local seed used by RANDOMIZE. `INP` and `OUT` reach actual processor
port-I/O support in the assembly interface, so they are machine-dependent.

## Floating-point and assembly support

The arithmetic representation is a historical four-byte binary format,
not IEEE 754. It uses a biased exponent and sign/fraction bytes, with
roughly seven decimal digits of precision. `FPPKG.SRC` credits Cal Ohme
and retains a December 1973 "8008" heading, although its instructions
here are 8080 assembly.

`FPINT.SRC` adapts the PL/M calling convention to the arithmetic and
conversion packages, exposes CP/M monitor jumps and memory-copy helpers,
and dispatches floating-point functions. Its initial jumps to BUILD and
the interpreter, plus the `BEGIN` word, must be patched for the final
memory map.

`FPCONV.SRC` supplies textual and fixed-point conversions. `TRAN.SRC`
provides square root by iteration, trigonometric/hyperbolic functions,
exponential/logarithmic routines, and a random generator. `FPDATA.SRC`
provides shared accumulator/conversion storage, constants, and scratch
aliases. Some arithmetic routines increment only the low address byte,
and the data module requests `CSEG INPAGE`; placement and page alignment
are therefore meaningful, not incidental linker details.

## Operating instructions and compiler options

The principal manual describes:

```text
BASIC DEMO
RUN DEMO
```

The compiler reads `DEMO.BAS` and produces `DEMO.INT`; the monitor runs
that intermediate file. Appended notes use the alternative command names
`EBASIC`/`ERUN`, but the archived executables are named `BASIC.COM` and
`RUN.COM`. Neither spelling makes the generated `.INT` a standalone binary.

The resident scanner reads an optional `$` option string from `006DH`
(the second default filename area), allowing a command such as
`BASIC DEMO $E`. Its implemented switches are:

| Switch | Effect |
| --- | --- |
| `A` | Enable grammar-production listing. |
| `B` | Suppress the normal source listing, retaining error-oriented listing. |
| `C` | Suppress intermediate-file creation for syntax checking. |
| `D` | Disable the normally enabled lowercase-to-uppercase conversion. |
| `E` | Emit source-line tracking instructions for run-time diagnostics. |
| `F` | Route compiler character output through the CP/M list device. |

The manual's appendix lists only A–E and describes older source-toggle
behavior; `F` and command-line option handling are visible in the resident
source. For exact behavior, use the implementation rather than assuming
the manual and all source revisions match.

## Preservation and build limitations

The PL/M modules use `PUBLIC`/`EXTERNAL` interfaces and
`$INCLUDE(:F1:BASCOM.LIT)`, preserving an original compiler-device path.
The assembly modules use relocatable-segment, public-symbol, and external
reference directives. Rebuilding requires a compatible PL/M compiler,
8080 assembler/linker, appropriate memory placement, and the documented
entry/BDOS patches. No scripts or complete patch/link instructions are
included here.

`manual.txt` retains formatter commands, page markers, damaged characters,
and transcription errors. It includes a grammar and useful error-code
reference, but its historical bug notes and limits should not be treated
as verified behavior of every source or binary in this directory.
The compatibility tests execute the preserved compiler on the hosted path
to obtain fixtures. The preserved `RUN.COM` is not used as the C runtime
and has not been validated across all execution targets.
