/* machine.h - interface of machine.c (hosted and i386 builds) */
#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>
#include "cpm.h"

extern int machine_ref;                      /* run the original binaries */
extern uint8_t machine_ref_ccp[0x800];       /* ... loaded here by the host */
extern uint8_t machine_ref_bdos[0xE00];
extern unsigned long long machine_max_instr; /* runaway guard */
extern int machine_trace;                    /* log BDOS calls (hosted) */

/* After machine_boot(): run the system.  Does not return. */
void machine_run(void) NORETURN;

/* Extra platform services for the machine layer */
void plat_halt(uint16_t pc) NORETURN;   /* CPU halted (DI/HLT) */
void plat_limit(void) NORETURN;         /* instruction limit reached */

#endif
