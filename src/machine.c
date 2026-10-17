/*
 * machine.c - the 64K machine around the port, for the hosted and i386
 * builds (the native Z80 build has native.c instead).
 *
 * Owns mem[], the low-memory vectors, boot and warm boot, and the 8080 CPU
 * used to run transient programs.  When the machine runs in reference mode
 * (hosted only) the original CCP and BDOS binaries execute on that CPU
 * instead of the C port, with the same BIOS routines trapped at the BIOS
 * jump table.
 */
#include <stdint.h>
#include "cpm.h"
#include "plat.h"
#include "i8080.h"
#include "machine.h"

#ifdef CPM_HOSTED
#include <stdio.h>
#endif

uint8_t mem[65536];

int machine_ref;
uint8_t machine_ref_ccp[0x800], machine_ref_bdos[0xE00];
unsigned long long machine_max_instr = 2000000000ULL;
int machine_trace;

static unsigned long long n_instr;
static cpm_jmpbuf boot_jmp;

static void mem_clear(void)
{
    uint32_t i;
    for (i = 0; i < sizeof mem; i++)
        mem[i] = 0;
}

static void mem_copy(uint16_t dst, const uint8_t *src, uint16_t n)
{
    uint16_t i;
    for (i = 0; i < n; i++)
        mem[(uint16_t)(dst + i)] = src[i];
}

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
}

static void load_system(void)
{
    if (machine_ref) {
        mem_copy(CCP_BASE, machine_ref_ccp, sizeof machine_ref_ccp);
        mem_copy(BDOS_BASE, machine_ref_bdos, sizeof machine_ref_bdos);
    } else {
        ccp_load();
        bdos_init();
    }
}

void machine_boot(void)
{
    uint8_t i;
    mem_clear();
    for (i = 0; i < 17 * 3; i++)
        mem[BIOS_BASE + i] = 0xC9;
    low_memory(1);
    bios_setup_disks();
    load_system();
}

void machine_wboot(void)
{
#ifdef CPM_HOSTED
    if (machine_trace)
        fprintf(stderr, "wboot (mem[4]=%02X)\n", rb(DISKA));
#endif
    /* reload CCP and BDOS, re-initialise the jump vectors, enter the CCP */
    load_system();
    low_memory(0);
    cpm_longjmp(boot_jmp);
    for (;;)
        ;
}

/* The CCP's bad-serial-number trap (DI, HLT at the CCP base) */
void cpm_halt(void)
{
    plat_halt(CCP_BASE + 1);
}

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

/* Execute a BIOS jump-table entry (reference mode, or a transient program
   that calls the BIOS directly) */
static void bios_entry(uint8_t idx)
{
    switch (idx) {
    case 0:  machine_boot(); break;
    case 1:  machine_wboot(); break;
    case 2:  cpu.a = bios->consts(); break;
    case 3:  cpu.a = bios->conin(); break;
    case 4:  bios->conout(cpu.c); break;
    case 5:  bios->list(cpu.c); break;
    case 6:  bios->punch(cpu.c); break;
    case 7:  cpu.a = bios->reader(); break;
    case 8:  bios->home(); break;
    case 9:  set_hl(bios->seldsk(cpu.c, cpu.e)); break;
    case 10: bios->settrk(BC()); break;
    case 11: bios->setsec(BC()); break;
    case 12: bios->setdma(BC()); break;
    case 13: cpu.a = bios->read(); break;
    case 14: cpu.a = bios->write(cpu.c); break;
    case 15: cpu.a = bios->listst(); break;
    case 16: set_hl(bios->sectran(BC(), DE())); break;
    }
}

#ifdef CPM_HOSTED
static uint16_t pend_ret, pend_sp;   /* trace: awaiting a BDOS return */
static int pending;
#endif

/* Returns 1 if the instruction at pc was handled as a trap */
static int trap(void)
{
    uint16_t pc = cpu.pc;
    if (pc >= BIOS_BASE && pc < BIOS_BASE + 17 * 3 &&
        (pc - BIOS_BASE) % 3 == 0) {
        bios_entry((uint8_t)((pc - BIOS_BASE) / 3));
        cpu.pc = pop16();
        return 1;
    }
#ifdef CPM_HOSTED
    if (machine_trace && pc == BDOS_ENTRY && machine_ref) {
        fprintf(stderr, "bdos %02X %04X\n", cpu.c, DE());
        if (cpu.c != 2 && cpu.c != 6) {
            pending = 1;
            pend_ret = rw(cpu.sp);
            pend_sp = cpu.sp;
        }
    }
#endif
    if (!machine_ref && pc == BDOS_ENTRY) {
        uint16_t hl = bdos_call(cpu.c, DE());
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
#ifdef CPM_HOSTED
    if (pending && cpu.pc == pend_ret && cpu.sp == (uint16_t)(pend_sp + 2)) {
        pending = 0;
        fprintf(stderr, "  = %04X\n", (cpu.h << 8) | cpu.l);
    }
    if (machine_trace && machine_ref && (cpu.pc < 0x100 || cpu.pc == 0xDF5C))
        fprintf(stderr, "pc=%04X sp=%04X c=%02X\n", cpu.pc, cpu.sp, cpu.c);
#endif
    if (++n_instr > machine_max_instr)
        plat_limit();
    if (trap())
        return;
    i8080_step();
    if (cpu.halted)
        plat_halt(cpu.pc);
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

void (*ccp_run_tpa)(void);

void machine_run(void)
{
    ccp_run_tpa = run_tpa;
    cpm_setjmp(boot_jmp);        /* a warm boot lands here */
    if (machine_ref) {
        /* the BIOS "jmp ccp" with C = user/disk byte */
        cpu.c = rb(DISKA);
        cpu.pc = CCP_BASE;
        for (;;)
            step();
    }
    ccp_start(rb(DISKA));
}
