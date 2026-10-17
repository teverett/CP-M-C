/* geom.h - the three disk geometries of the test machine */
#ifndef GEOM_H
#define GEOM_H

#include <stdint.h>

typedef struct {
    uint16_t spt;
    uint8_t bsh, blm, exm;
    uint16_t dsm, drm;
    uint8_t al0, al1;
    uint16_t cks, off;
    uint16_t tracks;
    uint8_t skewed;              /* uses the 6-sector skew table */
} geom;

#define NGEOMS  3
#define NDRIVES 4                /* drive D is never mounted */

static const uint8_t skew6[26] = {
    1, 7, 13, 19, 25, 5, 11, 17, 23, 3, 9, 15, 21,
    2, 8, 14, 20, 26, 6, 12, 18, 24, 4, 10, 16, 22
};

static const geom geoms[NGEOMS] = {
    /* A: 8" single density IBM 3740 */
    { 26, 3, 7, 0, 242, 63, 0xC0, 0, 16, 2, 77, 1 },
    /* B: 2K blocks, 16-bit block pointers (maxall > 255) */
    { 64, 4, 15, 0, 299, 127, 0xC0, 0, 32, 1, 76, 0 },
    /* C: 4K blocks, extent mask 3, no directory checksums (CKS=0) */
    { 64, 5, 31, 3, 99, 63, 0x80, 0, 0, 1, 51, 0 },
};

#define GEOM_BYTES(g) ((unsigned long)(g)->tracks * (g)->spt * 128UL)

#endif
