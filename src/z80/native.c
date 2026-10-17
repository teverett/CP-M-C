/*
 * native.c - boot, warm boot and the BDOS entry glue for the native Z80
 * build.  Here the CPU's address space is the CP/M address space, so
 * transient programs simply run, and the BDOS entry at BDOS_BASE+6 is a
 * jump into the compiled BDOS (see crt0.s).
 */
#include <stdint.h>
#include "../cpm.h"
#include "../plat.h"

#define BDOS_STACK 512

uint8_t  ent_c;                  /* registers at BDOS entry, set by crt0.s */
uint16_t ent_de, ent_hl, ent_sp;
uint8_t  bdos_stack[BDOS_STACK]; /* the BDOS runs on its own stack */

void (*ccp_run_tpa)(void);

extern void bdos_entry(void);    /* crt0.s */
extern void wboot_entry(void);
extern void bios_stub(void);
extern void ccp_entry(void);
extern void ccpclear_entry(void);

void bdos_entry_c(void)
{
    ent_hl = bdos_call(ent_c, ent_de);
}

static void jp(uint16_t at, void (*target)(void))
{
    wb(at, 0xC3);
    ww((uint16_t)(at + 1), (uint16_t)target);
}

static void low_memory(uint8_t cold)
{
    uint8_t i;
    wb(0, 0xC3);
    ww(1, BIOS_BASE + 3);
    if (cold) {
        wb(IOLOC, 0);
        wb(DISKA, 0);
    }
    wb(BDOSA, 0xC3);
    ww(6, BDOS_ENTRY);
    for (i = 0; i < 6; i++)      /* the serial number block */
        wb((uint16_t)(CCP_SERIAL + i), 0);
    jp(BDOS_ENTRY, bdos_entry);
    jp(CCP_BASE, ccp_entry);               /* the CCP's two entry points */
    jp(CCP_BASE + 3, ccpclear_entry);
    jp(BIOS_BASE, wboot_entry);
    jp(BIOS_BASE + 3, wboot_entry);
    for (i = 2; i < 17; i++)
        jp((uint16_t)(BIOS_BASE + 3 * i), bios_stub);
}

static void run_tpa(void)
{
    ((void (*)(void))TRAN)();
}

/* called from crt0.s after the stack and data are set up */
void cpm_start(void)
{
    ccp_run_tpa = run_tpa;
    low_memory(1);
    bios_setup_disks();
    ccp_load();
    bdos_init();
    ccp_start(0);
}

/* called from ccp_entry / ccpclear_entry with C = user/disk byte in ent_c */
void ccp_entry_c(void)
{
    ccp_start(ent_c);
}

void ccpclear_entry_c(void)
{
    wb(CCP_COMLEN, 0);
    ccp_start(ent_c);
}

/* called from wboot_entry (crt0.s) with the stack reset */
void wboot_c(void)
{
    ccp_load();
    bdos_init();
    low_memory(0);
    ccp_start(rb(DISKA));
}

void machine_boot(void)
{
}

void machine_wboot(void)
{
    __asm
        jp      _wboot_entry
    __endasm;
}

void cpm_halt(void)
{
    __asm
        di
        halt
    __endasm;
    for (;;)
        ;
}
