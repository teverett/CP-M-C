/*
 * cpm.h - shared definitions for the C port of CP/M 2.2 CCP and BDOS.
 *
 * The port keeps the structure of the original 8080 source: one C function
 * per assembly routine, the same names, the same variables.  Anything the
 * original kept in the 64K address space that other code can see (FCBs,
 * the command buffer, disk parameter headers, allocation vectors, the DMA
 * buffer) is addressed through rb()/wb()/rw()/ww().
 *
 * The same sources build three ways:
 *   hosted   (-DCPM_HOSTED)  mem[] is a 64K array; transient programs run
 *                            on an 8080 emulator.  Used for testing.
 *   i386     (-DCPM_I386)    as hosted, but freestanding: a bare-metal
 *                            kernel, run under QEMU.
 *   z80      (SDCC)          native: the CPU's own address space is the
 *                            CP/M address space; programs run directly.
 */
#ifndef CPM_H
#define CPM_H

#include <stdint.h>

#if defined(__SDCC)
#define CPM_NATIVE 1
#define NORETURN
#elif defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

/* ---- memory map and access --------------------------------------- */

#ifdef CPM_NATIVE
/* The compiled code and data live above BDOS_BASE + 0x100 */
#define CCP_BASE   0xA000u      /* CCP data area (TPA ends here) */
#define BDOS_BASE  0xA800u      /* serial number block; entry JMP is +6 */
#define BIOS_BASE  0xA880u      /* BIOS jump table */
#define BIOS_DATA  0xF300u      /* DPHs, DPBs, vectors, directory buffer */

#define rb(a)    (*(volatile uint8_t *)(uint16_t)(a))
#define wb(a, v) (*(volatile uint8_t *)(uint16_t)(a) = (uint8_t)(v))
#define rw(a)    (*(volatile uint16_t *)(uint16_t)(a))
#define ww(a, v) (*(volatile uint16_t *)(uint16_t)(a) = (uint16_t)(v))
#else
/* 56K system layout, as in the DRI "MOVCPM 56" memory map */
#define CCP_BASE   0xDC00u
#define BDOS_BASE  0xE400u
#define BIOS_BASE  0xF200u
#define BIOS_DATA  0xF300u

extern uint8_t mem[65536];

static inline uint8_t  rb(uint16_t a) { return mem[a]; }
static inline void     wb(uint16_t a, uint8_t v) { mem[a] = v; }
static inline uint16_t rw(uint16_t a)
{
    return (uint16_t)(mem[a] | (mem[(uint16_t)(a + 1)] << 8));
}
static inline void     ww(uint16_t a, uint16_t v)
{
    mem[a] = (uint8_t)v;
    mem[(uint16_t)(a + 1)] = (uint8_t)(v >> 8);
}
#endif

#define BDOS_ENTRY (BDOS_BASE + 6)

/* ---- non-local jumps ----------------------------------------------- */

#if defined(CPM_HOSTED) || defined(__SDCC)
#include <setjmp.h>
typedef jmp_buf cpm_jmpbuf;
#define cpm_setjmp(b)  setjmp(b)
#define cpm_longjmp(b) longjmp(b, 1)
#else
typedef void *cpm_jmpbuf[5];
#define cpm_setjmp(b)  __builtin_setjmp(b)
#define cpm_longjmp(b) __builtin_longjmp(b, 1)
#endif

/* Low memory */
#define IOLOC       0x0003
#define DISKA       0x0004
#define BDOSA       0x0005
#define TBUFF       0x0080
#define TRAN        0x0100

/*
 * The BIOS, one member per entry of the 17-entry jump table.  Same
 * register conventions as the original, expressed as arguments/results.
 * wboot never returns.
 */
typedef struct cpm_bios {
    void     (*boot)(void);
    void     (*wboot)(void);
    uint8_t  (*consts)(void);               /* 0xFF ready, 0 not */
    uint8_t  (*conin)(void);
    void     (*conout)(uint8_t c);
    void     (*list)(uint8_t c);
    void     (*punch)(uint8_t c);
    uint8_t  (*reader)(void);
    void     (*home)(void);
    uint16_t (*seldsk)(uint8_t disk, uint8_t e);  /* DPH address or 0 */
    void     (*settrk)(uint16_t track);
    void     (*setsec)(uint16_t sector);
    void     (*setdma)(uint16_t addr);
    uint8_t  (*read)(void);                 /* 0 ok */
    uint8_t  (*write)(uint8_t type);        /* 0 normal, 1 dir, 2 new block */
    uint8_t  (*listst)(void);
    uint16_t (*sectran)(uint16_t sector, uint16_t xlt);
} cpm_bios;

extern const cpm_bios bios_ops;
#define bios (&bios_ops)

/* ---- BDOS ---------------------------------------------------------- */

/* Entry from user programs: function in c, information address in de.
 * Returns HL; the 8080 caller gets A=L, B=H. */
uint16_t bdos_call(uint8_t c, uint16_t de);
#ifdef CPM_HOSTED
extern int bdos_trace;          /* debugging: log calls to stderr */
#endif

/* BDOS error handlers (the words at BDOS+9..+0Fh) */
typedef void (*bdos_errfn)(void);
extern bdos_errfn pererr, selerr, roderr, roferr;

/* Address of the one-byte "empty FCB" the BDOS searches with */
#define BDOS_EFCB  (BDOS_BASE + 0x20)

void bdos_init(void);            /* cold-start the BDOS variables */

/* ---- CCP ----------------------------------------------------------- */

/* Re-create the CCP image data, as a warm boot reload would. */
void ccp_load(void);
/* ccpstart: c = (user << 4) | disk.  Does not return. */
void ccp_start(uint8_t c) NORETURN;
/* Run the built-in CP/M context editor from the CCP command line. */
void ed_command(uint16_t command_buffer, uint8_t command_length,
                uint8_t current_drive);

/* Run the transient program loaded at TRAN until it returns (RET) or jumps
 * to location 0 (which warm boots, not returning). */
extern void (*ccp_run_tpa)(void);

/* The CCP found a bad serial number (the original halts the CPU) */
void cpm_halt(void) NORETURN;

/* Where CCP data lives in memory (offsets match the original layout) */
#define CCP_MAXLEN   (CCP_BASE + 6)
#define CCP_COMLEN   (CCP_BASE + 7)
#define CCP_COMBUF   (CCP_BASE + 8)
#define CCP_SERIAL   (BDOS_BASE)        /* six bytes the CCP compares */

/* ---- BIOS / machine ------------------------------------------------- */

void bios_setup_disks(void);     /* lay out DPH/DPB/vectors for mounted drives */
void machine_boot(void);         /* BIOS "boot" entry */
void machine_wboot(void) NORETURN;   /* BIOS "wboot" entry */

#endif
