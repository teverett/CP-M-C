# `orig/cpn86src` source survey

`orig/cpn86src/` contains nine assembly source files for the requester
(called the "slave" in the sources) side of CP/NET-86. The dated headers
identify version 1.00 from August and September 1983. This is an 8086,
segmented-memory networking extension to CP/M-86, not the 8080 CP/M 2.2
system in `orig/os/` or a network implementation for the C port in `src/os/`.

The directory contains a replacement command processor, a network BDOS
interceptor, a hardware-specific network interface, a loader, and three
operator utilities. It does not contain a server implementation, the
underlying CP/M-86 BDOS/BIOS, build scripts, or prebuilt system modules.
There is no build integration for these sources in this repository.

## File inventory

| File | Role |
| --- | --- |
| `CCPN.A86` | CP/NET-86 Console Command Processor: command input, resident commands, submit-file execution, and loading CP/M-86 transient programs. |
| `CPNETLDR.A86` | Loader that installs `CCPN.SYS`, `NDOS.SYS`, and `SNIOS.SYS` into the existing CP/M-86 system segment and transfers control to NDOS cold start. |
| `CPNETSTS.A86` | Displays requester ID, network status, drive mappings, and console/list-device mappings. |
| `LOGIN.A86` | Sends a login request with an eight-byte password and optional hexadecimal server ID, then marks the requester active on success. |
| `LOGOFF.A86` | Sends a logoff request to the selected server and clears the active status bit on success. |
| `NDOS.A86` | Network Disk Operating System: intercepts BDOS calls, routes local or remote operations, marshals file requests, and hooks selected BIOS entries. |
| `NDOSO.A86` | Alternate-origin NDOS source; otherwise identical to `NDOS.A86`. |
| `SNIOS.A86` | Slave Network Input Output System: configuration table, polled hardware I/O, message framing, checksums, acknowledgements, and retries. |
| `SNIOSO.A86` | Alternate-origin SNIOS source with a small banner change. |

The `O` copies are variants, not additional layers to load alongside their
counterparts:

- `NDOSO.A86` changes only `BASE EQU 0` to `BASE EQU 100H`.
- `SNIOSO.A86` makes the same origin change, adds `XYZ` to the startup
  banner, and has different trailing CP/M end-of-file padding.

The archive does not explain the intended build procedure for these variants.

## System organization and startup

The main call path is:

```text
CCPN or a CP/M-86 transient
            |
       INT E0h (BDOS)
            |
           NDOS
            +-- local operation --> original CP/M-86 BDOS/BIOS
            |
            +-- remote operation --> SNIOS --> network server
```

`CPNETLDR.A86` starts at offset `100H`. It checks the network-presence bit
(`0200H`) in the BDOS version result and refuses to load a second copy.
`GETMRG` requests the maximum available memory block and the BIOS memory
region table, requires that block to begin at the first region, and checks
the minimum space and system-offset limits.

The loader then reads `CCPN.SYS`, `NDOS.SYS`, and `SNIOS.SYS` in that order.
`LOAD` reads module length information, skips a further record, copies code
in 128-byte records, and applies a bitmapped relocation stream by adding the
load-offset high byte to marked bytes. Module destinations are rounded to
256-byte boundaries. This is a specific system-module loading scheme, not
simply concatenation of arbitrary `.CMD` files. The source prints the
resulting module and TPA locations and far-jumps to NDOS's cold-start entry.

NDOS cold start initializes SNIOS, obtains its configuration table, saves
the original BDOS entry and version, patches selected BIOS jump entries,
and redirects interrupt `E0H` to its own entry. It preserves the six-byte
identification area preceding the original BDOS entry and enters CCPN with
user zero and drive A selected.

The modules have positional dependencies:

- CCPN ends on a page boundary and exposes cold, abort, and hot entry jumps.
- NDOS ends on a page boundary and defines the SNIOS entry vector immediately
  after its own image.
- NDOS discovers CCPN relative to the BIOS memory region information and
  assumes known offsets within the CCP and underlying BDOS system data.
- Its BIOS jump-table assumption is offset `2500H` in the system segment;
  the loader expresses the same BIOS base as `250H` paragraphs.

These assumptions tie the sources to a particular CP/M-86 system layout.

## Console Command Processor (`CCPN.A86`)

CCPN provides six resident commands:

| Command | Behavior |
| --- | --- |
| `DIR` | Lists non-system directory entries. |
| `DIRS` | Lists system directory entries. |
| `ERA` | Deletes files, including wildcard handling and an all-files confirmation. |
| `TYPE` | Displays a text file, stopping at the CP/M text end marker. |
| `REN` | Renames a file, checking source existence, destination collisions, and drive consistency. |
| `USER` | Displays or changes the user number, with the set operation limited to 0–15. |

The command loop also supports drive changes, uppercases input, parses
8.3 file specifications and wildcards, and reports syntax and file errors.
`RDCOMN` reads queued commands from `$$$.SUB` on the home drive, consuming
the last record first and reducing the file's record count after each
command. Keyboard input can cancel submit execution.

Unknown commands are treated as transient names with the implicit extension
`.CMD`. `TRNCMD` opens the file, derives default attributes from its directory
entry, invokes BDOS function 59 to load it, prepares the default FCBs and
command tail in the program's base page, and calls its segmented entry
point. DMA offset and segment are managed separately. Program return leads
back through system reset rather than the 8080 `CALL 5`/`.COM` convention.

## Network BDOS (`NDOS.A86`)

### Dispatch and local fallback

`NDOSE` dispatches through three tables:

| Table | Purpose |
| --- | --- |
| `IXFUNC` | Maps BDOS function numbers to local pass-through, unsupported status, or a network processing sequence. |
| `IXMACR` | Encodes sequences of processing steps; the high bit marks the last step. |
| `IXMAIN` | Maps those steps to assembly routines for request construction, transport, and result restoration. |

Calls handled by NDOS switch to its own stack and save the caller's data
segment, stack, and working registers. `CHKDSK` and `GETDPT` inspect the
drive configuration to decide whether file operations should go to the
original BDOS or a server. `TOBDOS` preserves the interrupt-style calling
convention when invoking the saved BDOS entry.

### File and device operations

The request machinery covers open/close/create/delete/rename, directory
searches, sequential and random record I/O, file size and random-record
updates, attributes, record locking, disk vectors, allocation information,
and disk parameter blocks. It copies FCB state into messages and restores
returned state; file data uses 128-byte records. Search state remembers
whether the active search is local or belongs to a particular server.
Drive reset/access/free operations translate requester drive bitmaps into
server drive bitmaps and group requests by server.

NDOS tracks the selected drive, user code, DMA offset, and DMA segment.
It also supports password/compatibility state and command chaining.
`CHKFNM` substitutes requester-ID hexadecimal digits into special `$$$`
name/type fields for remote temporary or submit files, and `RSTNAM` restores
the caller-visible form.

Selected BIOS routines are trapped for warm boot, console status/input/output,
list output/status, and the memory region table. Console and printer requests
can therefore be redirected even when the original BDOS performs character
I/O. Network list output is buffered; warm boot frees drives and resets the
network interface. The adjusted memory region table excludes the resident
network extension from available transient memory.

### Network-specific functions

| BDOS function | Implementation |
| --- | --- |
| 64 | Login to a server using a server-ID byte followed by eight password bytes. |
| 65 | Log off a server. |
| 66 | Send a caller-supplied message through SNIOS. |
| 67 | Receive a message through SNIOS. |
| 68 | Read network status. |
| 69 | Return the requester configuration table address. |
| 70 | Set compatibility attributes. |
| 71 | Retrieve server configuration data. |
| 106 | Set the default password; specially dispatched outside the ordinary 0–71 range. |

Function 12 reports network presence by setting bit `0200H` in the version
result. `IXFUNC` explicitly rejects functions 41, 44, and 46, plus the
unused 60–63 range. Other functions, including memory allocation and program
loading, pass through to the original system where the table specifies zero.

`RPROCC` interprets server error replies. The saved BDOS error mode controls
whether NDOS returns silently, prints an error and returns, or prints an error
and warm-boots. Transport failures are returned as `FFH` through the NDOS
error-return path.

## Network interface (`SNIOS.A86`)

### Entry vector and configuration

SNIOS exposes eight three-byte jump entries:

| Offset | Entry | Purpose |
| --- | --- | --- |
| `+0` | `NTWKIN` | Initialize the network interface and startup state. |
| `+3` | `NTWKST` | Return status, clearing the receive/send error bits after reading. |
| `+6` | `CNTBAD` | Return the configuration table address. |
| `+9` | `SNDMSG` | Send a message addressed by `DS:CX`. |
| `+12` | `RCVMSG` | Receive into the message buffer addressed by `DS:CX`. |
| `+15` | `NTRSET` | Reset the hardware interface. |
| `+18` | `NTWKWB` | Network warm-boot reset. |
| `+21` | `NTWKEN` | Return the end of the resident SNIOS image. |

The configuration table starts with a network status byte, followed by the
requester ID, sixteen two-byte drive mappings, console and list mappings,
and list-output buffering state. NDOS usually keeps a pointer to the ID byte,
so its documented offsets are one byte lower than offsets measured from the
start of SNIOS's `CNFTBL`.

Each mapping contains a flag/device byte and a server-ID byte. Bit 7 selects
a network device and the low nibble identifies the server-side drive/device.
The NDOS layout comment also describes bit 6 as disk setup state.

The supplied configuration uses requester ID `10H`. Drives A–D and K–P are
local; E–J map to server `00H` drives A–F. Console and list devices default
to local. Status bits include active/logged-in (`10H`), receive error (`02H`),
and send error (`01H`); NDOS also uses bit `04H` for Control-P echo state.

### Transport and hardware assumptions

Messages have a five-byte header:

```text
FMT | DID | SID | FNC | SIZ | data...
```

`FMT` distinguishes request/reply format, `DID` and `SID` are destination and
source IDs, `FNC` is the function number, and `SIZ` encodes data length minus
one. NDOS reserves a 256-byte message data area.

The wire protocol uses ENQ/ACK negotiation, SOH plus the header and its
checksum, a second acknowledgement (`ACKS = 07H`), STX plus data, ETX,
a data checksum, EOT, and a final acknowledgement. Checksums are additive
byte sums with a transmitted complement that makes the checked total zero.
The send path includes handling for simultaneous send attempts.

This is direct, polled port I/O, not Ethernet, TCP/IP, or a host socket API:

| Constant | Value / assumption |
| --- | --- |
| `COMPRT`, `STATI`, `STATO` | Status/control port `D0H`. |
| `DPRTI`, `DPRTO` | Data port `D2H`. |
| `MSKI` | Input-ready test mask `01H`; a clear bit is treated as ready. |
| `MSKO` | Output-ready test mask `80H`; a clear bit is treated as ready. |
| `TMORTY` | 100 timeout polling attempts. |
| `MAXRTY` | 10 retries within a send/receive retry group. |

Timing loops are annotated for a 5 MHz CPU and 192K-baud communication.
Block reception temporarily disables interrupts. The retry-group constants
are not an overall operation deadline: some timeout paths set an error bit,
reset the interface, and restart the retry group. Adapting this module to
another machine requires understanding the original interface's control bits,
timing, and reset behavior.

## Operator utilities

`CPNETSTS.A86` checks that CP/NET is loaded, gets the configuration table and
network status, and displays all sixteen drive mappings plus console and list
assignments. Because status is obtained through SNIOS, reading it also clears
the transient send/receive error bits.

`LOGIN.A86` accepts a password and an optional bracketed hexadecimal server
ID, conceptually `LOGIN password[server-id]`. With no arguments, the source
uses server `00H` and a built-in eight-character default password. With only
a server selector, it retains that default password. This is a literal
command-tail parser, not a modern interactive credential prompt. On successful
function 64 return, it sets the active bit in the configuration table.

`LOGOFF.A86` accepts an optional `[server-id]`, defaults to server `00H`,
calls function 65, and clears the active bit on success. Both utilities
diagnose missing CP/NET, malformed commands, and failed requests.

## Build and preservation notes

The sources use Digital Research-style `.A86` syntax: `CSEG`, `DSEG`, `ORG`,
`RS`/`RB`/`RW`, segment overrides, and far calls/jumps. Several files retain
CP/M `1AH` end markers; comments also retain translated 8080 register names,
typographical errors, and historical terminology.

The loader names the expected `.SYS` artifacts, but the directory provides
neither the commands/toolchain to produce their headers and relocation maps
nor the matching CP/M-86 base system and hardware description. The exact
assembler, linker, and system-module conversion procedure cannot be established
from these nine files alone. In particular, the repository's 8080 assembler
and Z80 C utility build rules are not a build path for this archive.

The listing is useful as a reference for CP/NET requester routing, BDOS request
marshalling, segmented CP/M-86 program loading, and the boundary between
machine-independent NDOS behavior and hardware-specific SNIOS transport.
It should not be treated as a complete, directly runnable CP/NET-86 release.
