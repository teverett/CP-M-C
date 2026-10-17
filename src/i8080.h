/* i8080.h - small Intel 8080 CPU core */
#ifndef I8080_H
#define I8080_H

#include <stdint.h>

typedef struct {
    uint8_t a, b, c, d, e, h, l;
    uint8_t s, z, ac, p, cy;
    uint16_t sp, pc;
    int inte;
    int halted;
} i8080;

extern i8080 cpu;

/* Execute one instruction (memory is the global mem[]). */
void i8080_step(void);

#endif
