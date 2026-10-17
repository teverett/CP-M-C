/*
 * host.c - a small host system around the CP/M C port.
 *
 * Provides what the BIOS of a real machine would: console on stdin/stdout,
 * disk drives backed by image files, the low-memory jump vectors, and an
 * 8080 CPU for running transient (.COM) programs.
 *
 * The same host can instead run the *original* 8080 CCP and BDOS binaries
 * (assembled from orig/ccp.asm and orig/bdos.asm) on the emulated CPU, with the very same BIOS
 * routines trapped at the BIOS jump table.  That is how the C port is
 * tested: identical input must give identical console output and identical
 * disk images in both modes.
 *
 * usage: cpm [-r DIR] [-t] [-e TRK] [-m N] [-i FILE] [-l FILE] [-A IMG] [-B IMG]
 *            [-C IMG] [-D IMG]
 *   -r DIR   run original binaries DIR/ccp.bin, DIR/bdos.bin (reference)
 *   -t       console status reports type-ahead (input pending)
 *   -e TRK   disk reads/writes on track TRK fail (BDOS "Bad Sector")
 *   -x N:D:OFF:V  after N disk reads, set byte OFF of drive D's image to V
 *   -m N     abort after N emulated instructions (default 2e9)
 *   -i FILE  console input (default stdin); LF is delivered as CR
 *   -l FILE  list device output
 *   -A..-D   disk images; created (formatted) if missing, saved at exit
 */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpm.h"
#include "i8080.h"

uint8_t mem[65536];

/* ------------------------------------------------------------------ */
/* configuration                                                       */

static int ref_mode;
static int typeahead;
static struct {                 /* -x: change an image byte mid-run */
    long count, reads;
    int drive;
    size_t off;
    uint8_t val;
} inject = { -1, 0, 0, 0, 0 };
static int trace;               /* log BDOS calls to stderr */
static int err_track = -1;      /* BIOS read/write fail on this track */
static unsigned long long max_instr = 2000000000ULL;
static unsigned long long n_instr;
static uint8_t ref_ccp[0x800], ref_bdos[0xE00];

/* console input */
static uint8_t *inbuf;
static size_t inlen, inpos;

static FILE *listfp;

/* ------------------------------------------------------------------ */
/* disks                                                               */

typedef struct {
    uint16_t spt;
    uint8_t bsh, blm, exm;
    uint16_t dsm, drm;
    uint8_t al0, al1;
    uint16_t cks, off;
    uint16_t tracks;
    const uint8_t *xlt;          /* 1-based sector translation, or NULL */
} geom;

static const uint8_t skew6[26] = {
    1, 7, 13, 19, 25, 5, 11, 17, 23, 3, 9, 15, 21,
    2, 8, 14, 20, 26, 6, 12, 18, 24, 4, 10, 16, 22
};

static const geom geoms[3] = {
    /* A: 8" single density IBM 3740 */
    { 26, 3, 7, 0, 242, 63, 0xC0, 0, 16, 2, 77, skew6 },
    /* B: 2K blocks, 16-bit block pointers (maxall > 255) */
    { 64, 4, 15, 0, 299, 127, 0xC0, 0, 32, 1, 76, NULL },
    /* C: 4K blocks, extent mask 3, no directory checksums (CKS=0) */
    { 64, 5, 31, 3, 99, 63, 0x80, 0, 0, 1, 51, NULL },
};

#define NDRIVES 4
#define DIRBUF  0xF300u
#define DPH0    0xF400u
#define DPB0    0xF440u
#define XLT0    0xF480u
#define CSV0    0xF500u
#define ALV0    0xF600u

typedef struct {
    char *path;
    uint8_t *data;
    size_t size;
    int dirty;
} disk;

static disk disks[NDRIVES];
static uint8_t cur_disk;
static uint16_t cur_trk, cur_sec, cur_dma;

static size_t disk_size(const geom *g)
{
    return (size_t)g->tracks * g->spt * 128;
}

static void disk_open(int d)
{
    disk *dk = &disks[d];
    size_t want = disk_size(&geoms[d]);
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

static void host_exit(int code) __attribute__((noreturn));
void host_halt(void) __attribute__((noreturn));
static void host_exit(int code)
{
    fflush(stdout);
    if (listfp)
        fclose(listfp);
    disk_save_all();
    exit(code);
}

/* The CCP's bad-serial-number trap (DI, HLT at the CCP base) */
void host_halt(void)
{
    fflush(stdout);
    fprintf(stderr, "cpm: halted at %04X\n", CCP_BASE + 1);
    host_exit(4);
}

/* Lay out the DPH/DPB/vectors for every mounted drive */
static void bios_setup_disks(void)
{
    int d, i;
    for (d = 0; d < NDRIVES; d++) {
        const geom *g;
        uint16_t dph = (uint16_t)(DPH0 + d * 16);
        uint16_t dpb = (uint16_t)(DPB0 + d * 16);
        if (d >= 3 || !disks[d].path)
            continue;
        g = &geoms[d];
        if (g->xlt) {
            for (i = 0; i < g->spt; i++)
                wb((uint16_t)(XLT0 + i), g->xlt[i]);
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

/* ------------------------------------------------------------------ */
/* BIOS entry points                                                   */

static void low_memory(int cold)
{
    wb(0, 0xC3);
    ww(1, BIOS_BASE + 3);
    if (cold) {
        wb(IOLOC, 0);
        wb(DISKA, 0);
    }
    wb(BDOSA, 0xC3);
    ww(6, BDOS_ENTRY);
    cur_dma = TBUFF;
}

static jmp_buf boot_jmp;

static void load_system(void)
{
    if (ref_mode) {
        memcpy(&mem[CCP_BASE], ref_ccp, sizeof ref_ccp);
        memcpy(&mem[BDOS_BASE], ref_bdos, sizeof ref_bdos);
    } else {
        ccp_load();
        bdos_init();
    }
}

static void b_boot(void)
{
    memset(mem, 0, sizeof mem);
    memset(&mem[BIOS_BASE], 0xC9, 17 * 3);
    low_memory(1);
    bios_setup_disks();
    load_system();
}

static void b_wboot(void)
{
    if (trace)
        fprintf(stderr, "wboot (mem[4]=%02X)\n", rb(DISKA));
    /* reload CCP and BDOS, re-initialise the jump vectors, enter the CCP */
    load_system();
    low_memory(0);
    longjmp(boot_jmp, 1);
}

static uint8_t b_const(void)
{
    if (typeahead && inpos < inlen)
        return 0xFF;
    return 0;
}

static uint8_t b_conin(void)
{
    uint8_t c;
    if (inpos >= inlen)
        host_exit(0);            /* end of script */
    c = inbuf[inpos++];
    if (c == '\n')
        c = '\r';
    return c;
}

static void b_conout(uint8_t c)
{
    putchar(c);
}

static void b_list(uint8_t c)
{
    if (listfp)
        fputc(c, listfp);
}

static void b_punch(uint8_t c) { (void)c; }
static uint8_t b_reader(void) { return 0x1A; }
static void b_home(void) { cur_trk = 0; }

static uint16_t b_seldsk(uint8_t d, uint8_t e)
{
    (void)e;
    if (d >= 3 || !disks[d].path)
        return 0;
    disk_open(d);
    cur_disk = d;
    return (uint16_t)(DPH0 + d * 16);
}

static void b_settrk(uint16_t t) { cur_trk = t; }
static void b_setsec(uint16_t s) { cur_sec = s; }
static void b_setdma(uint16_t a) { cur_dma = a; }

static uint8_t *sector_ptr(void)
{
    const geom *g = &geoms[cur_disk];
    size_t sec;
    if (cur_trk >= g->tracks || cur_sec < 1 || cur_sec > g->spt)
        return NULL;
    sec = (size_t)cur_trk * g->spt + (size_t)(cur_sec - 1);
    return disks[cur_disk].data + sec * 128;
}

static uint8_t b_read(void)
{
    uint8_t *p;
    int i;
    if (inject.count >= 0 && ++inject.reads == inject.count &&
        disks[inject.drive].data)
        disks[inject.drive].data[inject.off] = inject.val;
    p = sector_ptr();
    if (!p || cur_trk == err_track)
        return 1;
    for (i = 0; i < 128; i++)
        wb((uint16_t)(cur_dma + i), p[i]);
    return 0;
}

static uint8_t b_write(uint8_t type)
{
    uint8_t *p = sector_ptr();
    int i;
    (void)type;
    if (!p || cur_trk == err_track)
        return 1;
    for (i = 0; i < 128; i++)
        p[i] = rb((uint16_t)(cur_dma + i));
    disks[cur_disk].dirty = 1;
    return 0;
}

static uint8_t b_listst(void) { return 0xFF; }

static uint16_t b_sectran(uint16_t sec, uint16_t xlt)
{
    if (xlt == 0)
        return (uint16_t)(sec + 1);
    return rb((uint16_t)(xlt + sec));
}

static const cpm_bios host_bios = {
    b_boot, b_wboot, b_const, b_conin, b_conout, b_list, b_punch, b_reader,
    b_home, b_seldsk, b_settrk, b_setsec, b_setdma, b_read, b_write,
    b_listst, b_sectran
};

/* ------------------------------------------------------------------ */
/* CPU glue                                                            */

static uint16_t pop16(void)
{
    uint16_t v = rw(cpu.sp);
    cpu.sp += 2;
    return v;
}

#define BC() ((uint16_t)((cpu.b << 8) | cpu.c))
#define DE() ((uint16_t)((cpu.d << 8) | cpu.e))

static void set_hl(uint16_t v)
{
    cpu.h = v >> 8;
    cpu.l = (uint8_t)v;
}

/* Execute a BIOS jump-table entry (ref mode, or a transient program that
   calls the BIOS directly) */
static void bios_entry(int idx)
{
    uint16_t v;
    switch (idx) {
    case 0:  b_boot(); break;
    case 1:  b_wboot(); break;
    case 2:  cpu.a = b_const(); break;
    case 3:  cpu.a = b_conin(); break;
    case 4:  b_conout(cpu.c); break;
    case 5:  b_list(cpu.c); break;
    case 6:  b_punch(cpu.c); break;
    case 7:  cpu.a = b_reader(); break;
    case 8:  b_home(); break;
    case 9:  v = b_seldsk(cpu.c, cpu.e); set_hl(v); break;
    case 10: b_settrk(BC()); break;
    case 11: b_setsec(BC()); break;
    case 12: b_setdma(BC()); break;
    case 13: cpu.a = b_read(); break;
    case 14: cpu.a = b_write(cpu.c); break;
    case 15: cpu.a = b_listst(); break;
    case 16: v = b_sectran(BC(), DE()); set_hl(v); break;
    }
}

static uint16_t pend_ret, pend_sp;   /* trace: awaiting a BDOS return */
static int pending;

/* Returns 1 if the instruction at pc was handled as a trap */
static int trap(void)
{
    uint16_t pc = cpu.pc;
    if (pc >= BIOS_BASE && pc < BIOS_BASE + 17 * 3 &&
        (pc - BIOS_BASE) % 3 == 0) {
        bios_entry((pc - BIOS_BASE) / 3);
        cpu.pc = pop16();
        return 1;
    }
    if (trace && pc == BDOS_ENTRY && ref_mode) {
        fprintf(stderr, "bdos %02X %04X\n", cpu.c, DE());
        if (cpu.c != 2 && cpu.c != 6) {
            pending = 1;
            pend_ret = rw(cpu.sp);
            pend_sp = cpu.sp;
        }
    }
    if (!ref_mode && pc == BDOS_ENTRY) {
        uint16_t hl;
        hl = bdos_call(cpu.c, DE());
        cpu.a = (uint8_t)hl;
        cpu.b = hl >> 8;
        set_hl(hl);
        cpu.pc = pop16();
        return 1;
    }
    return 0;
}

static void step(void)
{
    if (pending && cpu.pc == pend_ret && cpu.sp == pend_sp + 2) {
        pending = 0;
        fprintf(stderr, "  = %04X\n", (cpu.h << 8) | cpu.l);
    }
    if (++n_instr > max_instr) {
        fflush(stdout);
        fprintf(stderr, "cpm: instruction limit reached\n");
        host_exit(3);
    }
    if (trace && ref_mode && (cpu.pc < 0x100 || cpu.pc == 0xDF5C))
        fprintf(stderr, "pc=%04X sp=%04X c=%02X\n", cpu.pc, cpu.sp, cpu.c);
    if (trap())
        return;
    i8080_step();
    if (cpu.halted) {
        fflush(stdout);
        fprintf(stderr, "cpm: halted at %04X\n", cpu.pc);
        host_exit(4);
    }
}

#define TPA_RETURN 0xE3F0u       /* return address pushed for "call tran" */
#define TPA_STACK  0xE3A9u       /* SP the original CCP has at "call tran" */

static void run_tpa(void)
{
    cpu.sp = TPA_STACK;
    ww(cpu.sp, TPA_RETURN);
    cpu.pc = TRAN;
    while (cpu.pc != TPA_RETURN) {
        /* the CCP's two entry points, reached by a jump from the program */
        if (cpu.pc == CCP_BASE)
            ccp_start(cpu.c);
        if (cpu.pc == CCP_BASE + 3) {
            wb(CCP_COMLEN, 0);
            ccp_start(cpu.c);
        }
        step();
    }
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
    if (!f || fread(ref_ccp, 1, sizeof ref_ccp, f) == 0) {
        perror(p);
        exit(2);
    }
    fclose(f);
    snprintf(p, sizeof p, "%s/bdos.bin", dir);
    f = fopen(p, "rb");
    if (!f || fread(ref_bdos, 1, sizeof ref_bdos, f) == 0) {
        perror(p);
        exit(2);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    const char *infile = NULL;
    int i;

    setvbuf(stdout, NULL, _IOFBF, 1 << 16);
    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-r") && i + 1 < argc) {
            ref_mode = 1;
            load_ref(argv[++i]);
        } else if (!strcmp(a, "-t")) {
            typeahead = 1;
        } else if (!strcmp(a, "-T")) {
            trace = bdos_trace = 1;
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
            max_instr = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(a, "-i") && i + 1 < argc) {
            infile = argv[++i];
        } else if (!strcmp(a, "-l") && i + 1 < argc) {
            listfp = fopen(argv[++i], "wb");
        } else if (a[0] == '-' && a[1] >= 'A' && a[1] <= 'D' && !a[2] &&
                   i + 1 < argc) {
            disks[a[1] - 'A'].path = argv[++i];
        } else {
            fprintf(stderr, "usage: cpm [-r DIR] [-t] [-e TRK] [-x SPEC] [-m N] "
                            "[-i FILE] [-l FILE] [-A IMG] [-B IMG] [-C IMG] [-D IMG]\n");
            return 2;
        }
    }
    inbuf = slurp(infile, &inlen);

    bios = &host_bios;
    ccp_run_tpa = run_tpa;

    b_boot();                    /* cold start */
    if (setjmp(boot_jmp) == 0) {
        /* first entry */
    }
    if (ref_mode) {
        /* the BIOS "jmp ccp" with C = user/disk byte */
        cpu.c = rb(DISKA);
        cpu.pc = CCP_BASE;
        for (;;)
            step();
    }
    ccp_start(rb(DISKA));
}
