/* z80.h - small Zilog Z80 CPU core (documented instruction set) */
#ifndef Z80_H
#define Z80_H

#include <stdint.h>

typedef struct {
    uint8_t a, f, b, c, d, e, h, l;
    uint8_t a2, f2, b2, c2, d2, e2, h2, l2;
    uint16_t ix, iy, sp, pc;
    uint8_t i, r, iff1, iff2, im;
    int halted;
} z80_t;

extern z80_t z80;

/* I/O port hooks (low 8 bits of the port address) */
extern uint8_t (*z80_in)(uint8_t port);
extern void (*z80_out)(uint8_t port, uint8_t value);

/* Execute one instruction (memory is the global mem[]). */
void z80_step(void);

#endif
