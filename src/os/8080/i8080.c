/* i8080.c - small Intel 8080 CPU core */
#include <stdint.h>
#include "../cpm.h"
#include "i8080.h"

i8080 cpu;

static uint8_t parity(uint8_t v)
{
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return (uint8_t)(~v & 1);
}

static uint8_t getr(int r)
{
    switch (r) {
    case 0: return cpu.b;
    case 1: return cpu.c;
    case 2: return cpu.d;
    case 3: return cpu.e;
    case 4: return cpu.h;
    case 5: return cpu.l;
    case 6: return rb((uint16_t)((cpu.h << 8) | cpu.l));
    default: return cpu.a;
    }
}

static void setr(int r, uint8_t v)
{
    switch (r) {
    case 0: cpu.b = v; break;
    case 1: cpu.c = v; break;
    case 2: cpu.d = v; break;
    case 3: cpu.e = v; break;
    case 4: cpu.h = v; break;
    case 5: cpu.l = v; break;
    case 6: wb((uint16_t)((cpu.h << 8) | cpu.l), v); break;
    default: cpu.a = v; break;
    }
}

static uint16_t getrp(int rp)
{
    switch (rp) {
    case 0: return (uint16_t)((cpu.b << 8) | cpu.c);
    case 1: return (uint16_t)((cpu.d << 8) | cpu.e);
    case 2: return (uint16_t)((cpu.h << 8) | cpu.l);
    default: return cpu.sp;
    }
}

static void setrp(int rp, uint16_t v)
{
    switch (rp) {
    case 0: cpu.b = v >> 8; cpu.c = (uint8_t)v; break;
    case 1: cpu.d = v >> 8; cpu.e = (uint8_t)v; break;
    case 2: cpu.h = v >> 8; cpu.l = (uint8_t)v; break;
    default: cpu.sp = v; break;
    }
}

static void szp(uint8_t v)
{
    cpu.s = v >> 7;
    cpu.z = v == 0;
    cpu.p = parity(v);
}

static void push16(uint16_t v)
{
    cpu.sp -= 2;
    ww(cpu.sp, v);
}

static uint16_t pop16(void)
{
    uint16_t v = rw(cpu.sp);
    cpu.sp += 2;
    return v;
}

static uint8_t fetch8(void)
{
    return mem[cpu.pc++];
}

static uint16_t fetch16(void)
{
    uint16_t v = rw(cpu.pc);
    cpu.pc += 2;
    return v;
}

static uint8_t getf(void)
{
    return (uint8_t)(cpu.s << 7 | cpu.z << 6 | cpu.ac << 4 | cpu.p << 2 |
                     2 | cpu.cy);
}

static void setf(uint8_t f)
{
    cpu.s = (f >> 7) & 1;
    cpu.z = (f >> 6) & 1;
    cpu.ac = (f >> 4) & 1;
    cpu.p = (f >> 2) & 1;
    cpu.cy = f & 1;
}

static void alu(int op, uint8_t v)
{
    unsigned r;
    uint8_t a = cpu.a;
    switch (op) {
    case 0:                                  /* add */
    case 1:                                  /* adc */
        r = (unsigned)a + v + (op == 1 ? cpu.cy : 0);
        cpu.ac = ((a ^ v ^ r) >> 4) & 1;
        cpu.cy = r > 0xFF;
        cpu.a = (uint8_t)r;
        szp(cpu.a);
        break;
    case 2:                                  /* sub */
    case 3:                                  /* sbb */
    case 7:                                  /* cmp */
        r = (unsigned)a - v - (op == 3 ? cpu.cy : 0);
        cpu.ac = ((a ^ v ^ r) >> 4) & 1;
        cpu.cy = r > 0xFF;
        szp((uint8_t)r);
        if (op != 7)
            cpu.a = (uint8_t)r;
        break;
    case 4:                                  /* ana */
        cpu.ac = ((a | v) >> 3) & 1;
        cpu.a = a & v;
        cpu.cy = 0;
        szp(cpu.a);
        break;
    case 5:                                  /* xra */
        cpu.a = a ^ v;
        cpu.cy = cpu.ac = 0;
        szp(cpu.a);
        break;
    case 6:                                  /* ora */
        cpu.a = a | v;
        cpu.cy = cpu.ac = 0;
        szp(cpu.a);
        break;
    }
}

static int cond(int cc)
{
    switch (cc) {
    case 0: return !cpu.z;
    case 1: return cpu.z;
    case 2: return !cpu.cy;
    case 3: return cpu.cy;
    case 4: return !cpu.p;
    case 5: return cpu.p;
    case 6: return !cpu.s;
    default: return cpu.s;
    }
}

void i8080_step(void)
{
    uint8_t op = fetch8();
    int r = (op >> 3) & 7;
    int rp = (op >> 4) & 3;
    unsigned t;
    uint16_t w;

    if (op >= 0x40 && op < 0x80) {
        if (op == 0x76) {
            cpu.halted = 1;
            cpu.pc--;
            return;
        }
        setr(r, getr(op & 7));
        return;
    }
    if (op >= 0x80 && op < 0xC0) {
        alu(r, getr(op & 7));
        return;
    }
    switch (op) {
    case 0x00: case 0x08: case 0x10: case 0x18:
    case 0x20: case 0x28: case 0x30: case 0x38:     /* nop */
        return;
    case 0x07:                                      /* rlc */
        cpu.cy = cpu.a >> 7;
        cpu.a = (uint8_t)((cpu.a << 1) | cpu.cy);
        return;
    case 0x0F:                                      /* rrc */
        cpu.cy = cpu.a & 1;
        cpu.a = (uint8_t)((cpu.a >> 1) | (cpu.cy << 7));
        return;
    case 0x17:                                      /* ral */
        t = cpu.a >> 7;
        cpu.a = (uint8_t)((cpu.a << 1) | cpu.cy);
        cpu.cy = (uint8_t)t;
        return;
    case 0x1F:                                      /* rar */
        t = cpu.a & 1;
        cpu.a = (uint8_t)((cpu.a >> 1) | (cpu.cy << 7));
        cpu.cy = (uint8_t)t;
        return;
    case 0x27: {                                    /* daa */
        uint8_t adj = 0, cy = cpu.cy;
        if (cpu.ac || (cpu.a & 0x0F) > 9)
            adj |= 0x06;
        if (cpu.cy || cpu.a > 0x99) {
            adj |= 0x60;
            cy = 1;
        }
        alu(0, adj);
        cpu.cy = cy;
        return;
    }
    case 0x2F:                                      /* cma */
        cpu.a = (uint8_t)~cpu.a;
        return;
    case 0x37:                                      /* stc */
        cpu.cy = 1;
        return;
    case 0x3F:                                      /* cmc */
        cpu.cy = !cpu.cy;
        return;
    case 0x01: case 0x11: case 0x21: case 0x31:     /* lxi */
        setrp(rp, fetch16());
        return;
    case 0x09: case 0x19: case 0x29: case 0x39:     /* dad */
        t = (unsigned)getrp(2) + getrp(rp);
        cpu.cy = t > 0xFFFF;
        setrp(2, (uint16_t)t);
        return;
    case 0x02: case 0x12:                           /* stax */
        wb(getrp(rp), cpu.a);
        return;
    case 0x0A: case 0x1A:                           /* ldax */
        cpu.a = rb(getrp(rp));
        return;
    case 0x22:                                      /* shld */
        ww(fetch16(), (uint16_t)((cpu.h << 8) | cpu.l));
        return;
    case 0x2A:                                      /* lhld */
        setrp(2, rw(fetch16()));
        return;
    case 0x32:                                      /* sta */
        wb(fetch16(), cpu.a);
        return;
    case 0x3A:                                      /* lda */
        cpu.a = rb(fetch16());
        return;
    case 0x03: case 0x13: case 0x23: case 0x33:     /* inx */
        setrp(rp, (uint16_t)(getrp(rp) + 1));
        return;
    case 0x0B: case 0x1B: case 0x2B: case 0x3B:     /* dcx */
        setrp(rp, (uint16_t)(getrp(rp) - 1));
        return;
    case 0x04: case 0x0C: case 0x14: case 0x1C:
    case 0x24: case 0x2C: case 0x34: case 0x3C: {   /* inr */
        uint8_t v = (uint8_t)(getr(r) + 1);
        cpu.ac = (v & 0x0F) == 0;
        setr(r, v);
        szp(v);
        return;
    }
    case 0x05: case 0x0D: case 0x15: case 0x1D:
    case 0x25: case 0x2D: case 0x35: case 0x3D: {   /* dcr */
        uint8_t v = (uint8_t)(getr(r) - 1);
        cpu.ac = (v & 0x0F) != 0x0F;
        setr(r, v);
        szp(v);
        return;
    }
    case 0x06: case 0x0E: case 0x16: case 0x1E:
    case 0x26: case 0x2E: case 0x36: case 0x3E:     /* mvi */
        setr(r, fetch8());
        return;
    case 0xC3: case 0xCB:                           /* jmp */
        cpu.pc = fetch16();
        return;
    case 0xC9: case 0xD9:                           /* ret */
        cpu.pc = pop16();
        return;
    case 0xCD: case 0xDD: case 0xED: case 0xFD:     /* call */
        w = fetch16();
        push16(cpu.pc);
        cpu.pc = w;
        return;
    case 0xE9:                                      /* pchl */
        cpu.pc = getrp(2);
        return;
    case 0xF9:                                      /* sphl */
        cpu.sp = getrp(2);
        return;
    case 0xE3:                                      /* xthl */
        w = rw(cpu.sp);
        ww(cpu.sp, getrp(2));
        setrp(2, w);
        return;
    case 0xEB:                                      /* xchg */
        t = cpu.d; cpu.d = cpu.h; cpu.h = (uint8_t)t;
        t = cpu.e; cpu.e = cpu.l; cpu.l = (uint8_t)t;
        return;
    case 0xF3: cpu.inte = 0; return;
    case 0xFB: cpu.inte = 1; return;
    case 0xD3: (void)fetch8(); return;              /* out */
    case 0xDB: (void)fetch8(); cpu.a = 0; return;   /* in */
    case 0xC1: case 0xD1: case 0xE1:                /* pop */
        setrp(rp, pop16());
        return;
    case 0xF1:                                      /* pop psw */
        w = pop16();
        cpu.a = w >> 8;
        setf((uint8_t)w);
        return;
    case 0xC5: case 0xD5: case 0xE5:                /* push */
        push16(getrp(rp));
        return;
    case 0xF5:                                      /* push psw */
        push16((uint16_t)((cpu.a << 8) | getf()));
        return;
    case 0xC6: case 0xCE: case 0xD6: case 0xDE:
    case 0xE6: case 0xEE: case 0xF6: case 0xFE:     /* alu immediate */
        alu(r, fetch8());
        return;
    }
    switch (op & 7) {
    case 0:                                         /* rcc */
        if (cond(r))
            cpu.pc = pop16();
        return;
    case 2:                                         /* jcc */
        w = fetch16();
        if (cond(r))
            cpu.pc = w;
        return;
    case 4:                                         /* ccc */
        w = fetch16();
        if (cond(r)) {
            push16(cpu.pc);
            cpu.pc = w;
        }
        return;
    case 7:                                         /* rst */
        push16(cpu.pc);
        cpu.pc = (uint16_t)(op & 0x38);
        return;
    }
}
