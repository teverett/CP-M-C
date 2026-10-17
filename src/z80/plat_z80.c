/*
 * plat_z80.c - platform services for the native Z80 build: the console
 * and disk "hardware" is a set of I/O ports (see z80_ports.h).
 */
#include <stdint.h>
#include "../cpm.h"
#include "../plat.h"
#include "z80_ports.h"

__sfr __at (PORT_CON)   p_con;
__sfr __at (PORT_CONST) p_const;
__sfr __at (PORT_LIST)  p_list;
__sfr __at (PORT_PUNCH) p_punch;
__sfr __at (PORT_DRIVE) p_drive;
__sfr __at (PORT_TRKL)  p_trkl;
__sfr __at (PORT_TRKH)  p_trkh;
__sfr __at (PORT_SECL)  p_secl;
__sfr __at (PORT_SECH)  p_sech;
__sfr __at (PORT_DMAL)  p_dmal;
__sfr __at (PORT_DMAH)  p_dmah;
__sfr __at (PORT_CMD)   p_cmd;
__sfr __at (PORT_EXIT)  p_exit;

void plat_putc(uint8_t c)   { p_con = c; }
uint8_t plat_getc(void)     { return p_con; }
uint8_t plat_kbhit(void)    { return p_const; }
void plat_list(uint8_t c)   { p_list = c; }
void plat_punch(uint8_t c)  { p_punch = c; }
uint8_t plat_reader(void)   { return p_punch; }

int plat_disk_present(uint8_t d)
{
    p_drive = d;
    return p_drive;
}

uint8_t plat_disk_rw(int write, uint8_t d, uint16_t trk, uint16_t sec,
                     uint16_t dma)
{
    p_drive = d;
    p_trkl = (uint8_t)trk;
    p_trkh = (uint8_t)(trk >> 8);
    p_secl = (uint8_t)sec;
    p_sech = (uint8_t)(sec >> 8);
    p_dmal = (uint8_t)dma;
    p_dmah = (uint8_t)(dma >> 8);
    p_cmd = (uint8_t)write;
    return p_cmd;
}

void plat_exit(int code)
{
    p_exit = (uint8_t)code;
    for (;;)
        ;
}
