/*
 * plat.h - what the portable BIOS (bios.c) needs from the platform.
 *
 * Implemented by: host.c (hosted test machine), z80/plat_z80.c (native Z80,
 * via I/O ports) and plat_i386.c (bare-metal PC under QEMU).
 */
#ifndef PLAT_H
#define PLAT_H

#include <stdint.h>
#include "cpm.h"

void    plat_putc(uint8_t c);            /* console out */
uint8_t plat_getc(void);                 /* console in; blocks */
uint8_t plat_kbhit(void);                /* nonzero if input is waiting */
void    plat_list(uint8_t c);
void    plat_punch(uint8_t c);
uint8_t plat_reader(void);

/* Is drive d (0..2) present? */
int     plat_disk_present(uint8_t d);
/* Transfer one 128-byte sector between the drive and mem[dma..].
 * sec is the physical sector number as returned by SECTRAN (1-based).
 * Returns 0 on success, nonzero on error. */
uint8_t plat_disk_rw(int write, uint8_t d, uint16_t trk, uint16_t sec,
                     uint16_t dma);

void    plat_exit(int code) NORETURN;

#endif
