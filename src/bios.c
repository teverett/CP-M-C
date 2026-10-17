/*
 * bios.c - portable BIOS for the test machine.
 *
 * Console and sector I/O go through plat.h; everything else (disk
 * parameter headers and blocks, sector translation, current track/sector)
 * is here.  Drives A..C are the three geometries in geom.h; drive D is
 * never present, so selecting it gives the BDOS "Select" error.
 */
#include <stdint.h>
#include "cpm.h"
#include "plat.h"
#include "geom.h"

#define DIRBUF  (BIOS_DATA + 0x000)
#define DPH0    (BIOS_DATA + 0x100)
#define DPB0    (BIOS_DATA + 0x140)
#define XLT0    (BIOS_DATA + 0x180)
#define CSV0    (BIOS_DATA + 0x200)
#define ALV0    (BIOS_DATA + 0x300)

static uint8_t  cur_disk;
static uint16_t cur_trk, cur_sec, cur_dma;

/* Lay out the DPH/DPB/vectors for every mounted drive */
void bios_setup_disks(void)
{
    uint8_t d, i;
    for (d = 0; d < NGEOMS; d++) {
        const geom *g = &geoms[d];
        uint16_t dph = (uint16_t)(DPH0 + d * 16);
        uint16_t dpb = (uint16_t)(DPB0 + d * 16);
        if (!plat_disk_present(d))
            continue;
        if (g->skewed) {
            for (i = 0; i < 26; i++)
                wb((uint16_t)(XLT0 + i), skew6[i]);
            ww(dph, XLT0);
        } else {
            ww(dph, 0);
        }
        for (i = 2; i < 8; i++)
            wb((uint16_t)(dph + i), 0);
        ww((uint16_t)(dph + 8), DIRBUF);
        ww((uint16_t)(dph + 10), dpb);
        ww((uint16_t)(dph + 12), (uint16_t)(CSV0 + d * 0x40));
        ww((uint16_t)(dph + 14), (uint16_t)(ALV0 + d * 0x40));
        ww(dpb, g->spt);
        wb((uint16_t)(dpb + 2), g->bsh);
        wb((uint16_t)(dpb + 3), g->blm);
        wb((uint16_t)(dpb + 4), g->exm);
        ww((uint16_t)(dpb + 5), g->dsm);
        ww((uint16_t)(dpb + 7), g->drm);
        wb((uint16_t)(dpb + 9), g->al0);
        wb((uint16_t)(dpb + 10), g->al1);
        ww((uint16_t)(dpb + 11), g->cks);
        ww((uint16_t)(dpb + 13), g->off);
    }
}

static uint8_t b_const(void)
{
    return plat_kbhit() ? 0xFF : 0;
}

static uint8_t b_conin(void)
{
    uint8_t c = plat_getc();
    if (c == '\n')
        c = '\r';                /* the test scripts use LF for Enter */
    return c;
}

static void b_conout(uint8_t c)   { plat_putc(c); }
static void b_list(uint8_t c)     { plat_list(c); }
static void b_punch(uint8_t c)    { plat_punch(c); }
static uint8_t b_reader(void)     { return plat_reader(); }
static void b_home(void)          { cur_trk = 0; }

static uint16_t b_seldsk(uint8_t d, uint8_t e)
{
    (void)e;
    if (d >= NGEOMS || !plat_disk_present(d))
        return 0;
    cur_disk = d;
    return (uint16_t)(DPH0 + d * 16);
}

static void b_settrk(uint16_t t)  { cur_trk = t; }
static void b_setsec(uint16_t s)  { cur_sec = s; }
static void b_setdma(uint16_t a)  { cur_dma = a; }

static uint8_t b_read(void)
{
    return plat_disk_rw(0, cur_disk, cur_trk, cur_sec, cur_dma);
}

static uint8_t b_write(uint8_t type)
{
    (void)type;
    return plat_disk_rw(1, cur_disk, cur_trk, cur_sec, cur_dma);
}

static uint8_t b_listst(void)     { return 0xFF; }

static uint16_t b_sectran(uint16_t sec, uint16_t xlt)
{
    if (xlt == 0)
        return (uint16_t)(sec + 1);
    return rb((uint16_t)(xlt + sec));
}

const cpm_bios bios_ops = {
    machine_boot, machine_wboot, b_const, b_conin, b_conout, b_list,
    b_punch, b_reader, b_home, b_seldsk, b_settrk, b_setsec, b_setdma,
    b_read, b_write, b_listst, b_sectran
};
