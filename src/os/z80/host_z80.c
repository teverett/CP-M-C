/*
 * host_z80.c - the virtual Z80 machine: runs a natively compiled system
 * image (built with SDCC, see the z80 target in the Makefile) on the Z80
 * core.  The devices behind the I/O ports of z80_ports.h are the same
 * console and disk services the other builds use (plat_*).
 */
#include <stdio.h>
#include <stdlib.h>
#include "../cpm.h"
#include "../plat.h"
#include "../machine.h"
#include "z80.h"
#include "z80_ports.h"
#include "../host.h"

static uint8_t drive;
static uint16_t trk, sec, dma;
static uint8_t cmd_status;

static uint8_t zin(uint8_t port)
{
    switch (port) {
    case PORT_CON:    return plat_getc();
    case PORT_CONST:  return plat_kbhit() ? 0xFF : 0;
    case PORT_PUNCH:  return plat_reader();
    case PORT_DRIVE:  return (uint8_t)plat_disk_present(drive);
    case PORT_CMD:    return cmd_status;
    default:          return 0xFF;
    }
}

static void zout(uint8_t port, uint8_t v)
{
    switch (port) {
    case PORT_CON:    plat_putc(v); break;
    case PORT_LIST:   plat_list(v); break;
    case PORT_PUNCH:  plat_punch(v); break;
    case PORT_DRIVE:  drive = v; break;
    case PORT_TRKL:   trk = (uint16_t)((trk & 0xFF00) | v); break;
    case PORT_TRKH:   trk = (uint16_t)((trk & 0x00FF) | (v << 8)); break;
    case PORT_SECL:   sec = (uint16_t)((sec & 0xFF00) | v); break;
    case PORT_SECH:   sec = (uint16_t)((sec & 0x00FF) | (v << 8)); break;
    case PORT_DMAL:   dma = (uint16_t)((dma & 0xFF00) | v); break;
    case PORT_DMAH:   dma = (uint16_t)((dma & 0x00FF) | (v << 8)); break;
    case PORT_CMD:    cmd_status = plat_disk_rw(v & 1, drive, trk, sec, dma);
                      break;
    case PORT_EXIT:   plat_exit(v); break;
    default:          break;
    }
}

void host_run_z80(const char *image)
{
    FILE *f = fopen(image, "rb");
    size_t n;
    unsigned long long count = 0;

    if (!f) {
        perror(image);
        exit(2);
    }
    n = fread(mem, 1, sizeof mem, f);
    fclose(f);
    if (n == 0) {
        fprintf(stderr, "cpm: empty system image\n");
        exit(2);
    }
    z80_in = zin;
    z80_out = zout;
    z80.pc = 0;
    for (;;) {
        if (++count > machine_max_instr)
            plat_limit();
        z80_step();
        if (z80.halted)
            plat_halt(z80.pc);
    }
}
