/*
 * boot_hdr.h - layout of the QEMU system disk used by the i386 build.
 *
 * The IDE disk is a raw image:
 *   LBA 0                 header (struct boot_hdr) - parameters for the run
 *   LBA DRIVE_LBA(d)      CP/M drive d (A, B, C), up to DRIVE_SECTORS sectors
 *   LBA SCRIPT_LBA...     console script (script_len bytes), if any
 * With script_len == NO_SCRIPT the console is the serial port (interactive).
 */
#ifndef BOOT_HDR_H
#define BOOT_HDR_H

#include <stdint.h>

#define BOOT_MAGIC     0x434D5043u     /* "CPMC" */
#define NO_SCRIPT      0xFFFFFFFFu
#define DRIVE_SECTORS  2048u           /* 1 MB slot per drive */
#define DRIVE_LBA(d)   (16u + (d) * DRIVE_SECTORS)
#define SCRIPT_LBA     (16u + 3u * DRIVE_SECTORS)

#define BF_TYPEAHEAD   1u

struct boot_hdr {
    uint32_t magic;
    uint32_t flags;
    int32_t  err_track;            /* -1: none */
    int32_t  inject_count;         /* -1: none */
    uint32_t inject_drive;
    uint32_t inject_off;
    uint32_t inject_val;
    uint32_t script_len;
    uint32_t present;              /* bit d: drive d present */
};

#endif
