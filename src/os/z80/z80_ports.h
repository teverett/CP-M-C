/*
 * z80_ports.h - I/O ports of the virtual Z80 machine.
 *
 * Shared by plat_z80.c (the native BIOS driver, built with SDCC) and
 * host_z80.c (the device model in the test host).
 */
#ifndef Z80_PORTS_H
#define Z80_PORTS_H

#define PORT_CON     0x00   /* OUT: console character; IN: next input char */
#define PORT_CONST   0x01   /* IN: 0xFF if input is waiting (type-ahead) */
#define PORT_LIST    0x02   /* OUT: list device character */
#define PORT_PUNCH   0x03   /* OUT: punch character; IN: reader character */

#define PORT_DRIVE   0x10   /* OUT: select drive; IN: 1 if it is present */
#define PORT_TRKL    0x11   /* OUT: track, low byte */
#define PORT_TRKH    0x12   /* OUT: track, high byte */
#define PORT_SECL    0x13   /* OUT: sector (1-based), low byte */
#define PORT_SECH    0x14   /* OUT: sector, high byte */
#define PORT_DMAL    0x15   /* OUT: DMA address, low byte */
#define PORT_DMAH    0x16   /* OUT: DMA address, high byte */
#define PORT_CMD     0x17   /* OUT: 0 read, 1 write; IN: status (0 = ok) */

#define PORT_EXIT    0xFF   /* OUT: leave the emulator with this status */

#endif
