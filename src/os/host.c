/*
 * host.c - the hosted test machine: console on stdin/stdout, disk drives
 * backed by image files, options for fault injection and tracing.
 *
 * It can run the C port, the original 8080 CCP and BDOS binaries
 * (assembled from orig/ccp.asm and orig/bdos.asm) on the emulated CPU
 * with the very same BIOS (-r), or a natively compiled Z80 system image
 * on a Z80 core (-z).  Identical input must give identical console
 * output and identical disk images in every mode; that is how the port is
 * tested.
 *
 * usage: cpm [-r DIR | -z SYSTEM.BIN] [-t] [-e TRK] [-x SPEC] [-m N]
 *            [-i FILE] [-l FILE] [-A IMG] [-B IMG] [-C IMG]
 *   -r DIR   run original binaries DIR/ccp.bin, DIR/bdos.bin (reference)
 *   -z FILE  run a native Z80 build (see the z80 target in the Makefile)
 *   -t       console status reports type-ahead (input pending)
 *   -e TRK   disk reads/writes on track TRK fail (BDOS "Bad Sector")
 *   -x N:D:OFF:V  after N disk reads, set byte OFF of drive D's image to V
 *   -m N     abort after N emulated instructions (default 2e9)
 *   -i FILE  console input (default stdin); LF is delivered as CR
 *   -l FILE  list device output
 *   -A..-C   disk images; created (formatted) if missing, saved at exit
 *   -T       trace BDOS calls to stderr
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpm.h"
#include "plat.h"
#include "geom.h"
#include "machine.h"
#include "host.h"

/* ------------------------------------------------------------------ */
/* console                                                             */

int host_typeahead;
static uint8_t *inbuf;
static size_t inlen, inpos;
static FILE *listfp;

/* ------------------------------------------------------------------ */
/* disks                                                               */

typedef struct {
    char *path;
    uint8_t *data;
    size_t size;
    int dirty;
} disk;

static disk disks[NDRIVES];

static int err_track = -1;       /* fail disk I/O on this track */
static struct {                  /* -x: change an image byte mid-run */
    long count, reads;
    int drive;
    size_t off;
    uint8_t val;
} inject = { -1, 0, 0, 0, 0 };

static void disk_open(int d)
{
    disk *dk = &disks[d];
    size_t want = GEOM_BYTES(&geoms[d]);
    FILE *f;
    if (!dk->path || dk->data)
        return;
    dk->data = malloc(want);
    if (!dk->data) {
        perror("malloc");
        exit(2);
    }
    memset(dk->data, 0xE5, want);
    dk->size = want;
    f = fopen(dk->path, "rb");
    if (f) {
        size_t n = fread(dk->data, 1, want, f);
        (void)n;
        fclose(f);
    }
}

static void disk_save_all(void)
{
    int d;
    for (d = 0; d < NDRIVES; d++) {
        FILE *f;
        if (!disks[d].data || !disks[d].dirty)
            continue;
        f = fopen(disks[d].path, "wb");
        if (!f) {
            perror(disks[d].path);
            continue;
        }
        fwrite(disks[d].data, 1, disks[d].size, f);
        fclose(f);
    }
}

void plat_exit(int code)
{
    fflush(stdout);
    if (listfp)
        fclose(listfp);
    disk_save_all();
    exit(code);
}

void plat_halt(uint16_t pc)
{
    fflush(stdout);
    fprintf(stderr, "cpm: halted at %04X\n", pc);
    plat_exit(4);
}

void plat_limit(void)
{
    fflush(stdout);
    fprintf(stderr, "cpm: instruction limit reached\n");
    plat_exit(3);
}

/* ------------------------------------------------------------------ */
/* plat.h services                                                     */

void plat_putc(uint8_t c)
{
    putchar(c);
}

uint8_t plat_getc(void)
{
    if (inpos >= inlen)
        plat_exit(0);            /* end of script */
    return inbuf[inpos++];
}

uint8_t plat_kbhit(void)
{
    return host_typeahead && inpos < inlen;
}

void plat_list(uint8_t c)
{
    if (listfp)
        fputc(c, listfp);
}

void plat_punch(uint8_t c) { (void)c; }
uint8_t plat_reader(void) { return 0x1A; }

int plat_disk_present(uint8_t d)
{
    return d < NGEOMS && disks[d].path != NULL;
}

uint8_t plat_disk_rw(int write, uint8_t d, uint16_t trk, uint16_t sec,
                     uint16_t dma)
{
    const geom *g = &geoms[d];
    uint8_t *p;
    int i;

    if (!write && inject.count >= 0 && ++inject.reads == inject.count &&
        disks[inject.drive].data) {
        disks[inject.drive].data[inject.off] = inject.val;
        disks[inject.drive].dirty = 1;      /* the medium really changed */
    }
    disk_open(d);
    if (trk >= g->tracks || sec < 1 || sec > g->spt || trk == err_track)
        return 1;
    p = disks[d].data + ((size_t)trk * g->spt + (size_t)(sec - 1)) * 128;
    if (write) {
        for (i = 0; i < 128; i++)
            p[i] = rb((uint16_t)(dma + i));
        disks[d].dirty = 1;
    } else {
        for (i = 0; i < 128; i++)
            wb((uint16_t)(dma + i), p[i]);
    }
    return 0;
}

/* ------------------------------------------------------------------ */

static uint8_t *slurp(const char *path, size_t *len)
{
    FILE *f = path ? fopen(path, "rb") : stdin;
    size_t cap = 4096, n = 0, r;
    uint8_t *b = malloc(cap);
    if (!f) {
        perror(path);
        exit(2);
    }
    while ((r = fread(b + n, 1, cap - n, f)) > 0) {
        n += r;
        if (n == cap)
            b = realloc(b, cap *= 2);
    }
    if (path)
        fclose(f);
    *len = n;
    return b;
}

static void load_ref(const char *dir)
{
    char p[1024];
    FILE *f;
    snprintf(p, sizeof p, "%s/ccp.bin", dir);
    f = fopen(p, "rb");
    if (!f || fread(machine_ref_ccp, 1, sizeof machine_ref_ccp, f) == 0) {
        perror(p);
        exit(2);
    }
    fclose(f);
    snprintf(p, sizeof p, "%s/bdos.bin", dir);
    f = fopen(p, "rb");
    if (!f || fread(machine_ref_bdos, 1, sizeof machine_ref_bdos, f) == 0) {
        perror(p);
        exit(2);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    const char *infile = NULL, *zsystem = NULL;
    int i;

    setvbuf(stdout, NULL, _IOFBF, 1 << 16);
    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-r") && i + 1 < argc) {
            machine_ref = 1;
            load_ref(argv[++i]);
        } else if (!strcmp(a, "-z") && i + 1 < argc) {
            zsystem = argv[++i];
        } else if (!strcmp(a, "-t")) {
            host_typeahead = 1;
        } else if (!strcmp(a, "-T")) {
            machine_trace = bdos_trace = 1;
        } else if (!strcmp(a, "-e") && i + 1 < argc) {
            err_track = atoi(argv[++i]);
        } else if (!strcmp(a, "-x") && i + 1 < argc) {
            unsigned long off;
            unsigned val;
            if (sscanf(argv[++i], "%ld:%d:%lu:%u", &inject.count,
                       &inject.drive, &off, &val) != 4 ||
                inject.drive < 0 || inject.drive >= NDRIVES) {
                fprintf(stderr, "cpm: bad -x argument\n");
                return 2;
            }
            inject.off = off;
            inject.val = (uint8_t)val;
        } else if (!strcmp(a, "-m") && i + 1 < argc) {
            machine_max_instr = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(a, "-i") && i + 1 < argc) {
            infile = argv[++i];
        } else if (!strcmp(a, "-l") && i + 1 < argc) {
            listfp = fopen(argv[++i], "wb");
        } else if (a[0] == '-' && a[1] >= 'A' && a[1] <= 'D' && !a[2] &&
                   i + 1 < argc) {
            disks[a[1] - 'A'].path = argv[++i];
        } else {
            fprintf(stderr,
                    "usage: cpm [-r DIR | -z SYSTEM.BIN] [-t] [-e TRK] "
                    "[-x SPEC] [-m N] [-i FILE] [-l FILE] [-A IMG] [-B IMG] "
                    "[-C IMG]\n");
            return 2;
        }
    }
    inbuf = slurp(infile, &inlen);

    if (zsystem)
        host_run_z80(zsystem);   /* does not return */
    machine_boot();              /* cold start */
    machine_run();
}
