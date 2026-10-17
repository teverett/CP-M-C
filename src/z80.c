/*
 * z80.c - small Zilog Z80 CPU core.
 *
 * Implements the documented instruction set (plus the DD/FD CB forms) with
 * documented flag behaviour; the undocumented X/Y flag bits are tracked for
 * the common cases but nothing relies on them.  No interrupts, no timing.
 */
#include <stdint.h>
#include "cpm.h"
#include "z80.h"

z80_t z80;
uint8_t (*z80_in)(uint8_t port);
void (*z80_out)(uint8_t port, uint8_t value);

#define FS 0x80
#define FZ 0x40
#define FY 0x20
#define FH 0x10
#define FX 0x08
#define FP 0x04
#define FN 0x02
#define FC 0x01

#define Z z80

static uint8_t parity_tab[256];
static int parity_ready;

static uint8_t par(uint8_t v)
{
    if (!parity_ready) {
        int i, j;
        for (i = 0; i < 256; i++) {
            int n = 0;
            for (j = 0; j < 8; j++)
                n += (i >> j) & 1;
            parity_tab[i] = (n & 1) ? 0 : FP;
        }
        parity_ready = 1;
    }
    return parity_tab[v];
}

static uint8_t fetch8(void)
{
    return mem[Z.pc++];
}

static uint16_t fetch16(void)
{
    uint16_t v = rw(Z.pc);
    Z.pc += 2;
    return v;
}

static void push16(uint16_t v)
{
    Z.sp -= 2;
    ww(Z.sp, v);
}

static uint16_t pop16(void)
{
    uint16_t v = rw(Z.sp);
    Z.sp += 2;
    return v;
}

static uint16_t get_hl(void) { return (uint16_t)((Z.h << 8) | Z.l); }
static void set_hl(uint16_t v) { Z.h = v >> 8; Z.l = (uint8_t)v; }
static uint16_t get_bc(void) { return (uint16_t)((Z.b << 8) | Z.c); }
static void set_bc(uint16_t v) { Z.b = v >> 8; Z.c = (uint8_t)v; }
static uint16_t get_de(void) { return (uint16_t)((Z.d << 8) | Z.e); }
static void set_de(uint16_t v) { Z.d = v >> 8; Z.e = (uint8_t)v; }

/* register by 3-bit code; 6 is (HL) and handled by callers via addr */
static uint8_t get_r(int r, uint16_t addr)
{
    switch (r) {
    case 0: return Z.b;
    case 1: return Z.c;
    case 2: return Z.d;
    case 3: return Z.e;
    case 4: return Z.h;
    case 5: return Z.l;
    case 6: return rb(addr);
    default: return Z.a;
    }
}

static void set_r(int r, uint16_t addr, uint8_t v)
{
    switch (r) {
    case 0: Z.b = v; break;
    case 1: Z.c = v; break;
    case 2: Z.d = v; break;
    case 3: Z.e = v; break;
    case 4: Z.h = v; break;
    case 5: Z.l = v; break;
    case 6: wb(addr, v); break;
    default: Z.a = v; break;
    }
}

/* register pairs: 0 BC, 1 DE, 2 HL(IX/IY), 3 SP */
static uint16_t get_rp(int p, uint16_t *idx)
{
    switch (p) {
    case 0: return get_bc();
    case 1: return get_de();
    case 2: return idx ? *idx : get_hl();
    default: return Z.sp;
    }
}

static void set_rp(int p, uint16_t *idx, uint16_t v)
{
    switch (p) {
    case 0: set_bc(v); break;
    case 1: set_de(v); break;
    case 2: if (idx) *idx = v; else set_hl(v); break;
    default: Z.sp = v; break;
    }
}

/* ---- arithmetic -------------------------------------------------- */

static uint8_t add8(uint8_t a, uint8_t b, int c)
{
    unsigned r = (unsigned)a + b + c;
    uint8_t res = (uint8_t)r;
    Z.f = (uint8_t)((res & (FS | FY | FX)) | (res == 0 ? FZ : 0) |
          (((a ^ b ^ r) & 0x10) ? FH : 0) |
          ((~(a ^ b) & (a ^ res) & 0x80) ? FP : 0) | (r > 0xFF ? FC : 0));
    return res;
}

static uint8_t sub8(uint8_t a, uint8_t b, int c)
{
    unsigned r = (unsigned)a - b - c;
    uint8_t res = (uint8_t)r;
    Z.f = (uint8_t)((res & (FS | FY | FX)) | (res == 0 ? FZ : 0) |
          (((a ^ b ^ r) & 0x10) ? FH : 0) |
          (((a ^ b) & (a ^ res) & 0x80) ? FP : 0) | FN | (r > 0xFF ? FC : 0));
    return res;
}

static void alu(int op, uint8_t v)
{
    uint8_t a = Z.a;
    switch (op) {
    case 0: Z.a = add8(a, v, 0); break;
    case 1: Z.a = add8(a, v, Z.f & FC); break;
    case 2: Z.a = sub8(a, v, 0); break;
    case 3: Z.a = sub8(a, v, Z.f & FC); break;
    case 4:
        Z.a = a & v;
        Z.f = (uint8_t)((Z.a & (FS | FY | FX)) | (Z.a ? 0 : FZ) | FH |
                        par(Z.a));
        break;
    case 5:
        Z.a = a ^ v;
        Z.f = (uint8_t)((Z.a & (FS | FY | FX)) | (Z.a ? 0 : FZ) | par(Z.a));
        break;
    case 6:
        Z.a = a | v;
        Z.f = (uint8_t)((Z.a & (FS | FY | FX)) | (Z.a ? 0 : FZ) | par(Z.a));
        break;
    default:
        sub8(a, v, 0);
        Z.f = (uint8_t)((Z.f & ~(FY | FX)) | (v & (FY | FX)));
        break;
    }
}

static uint8_t inc8(uint8_t v)
{
    uint8_t r = (uint8_t)(v + 1);
    Z.f = (uint8_t)((Z.f & FC) | (r & (FS | FY | FX)) | (r ? 0 : FZ) |
          ((r & 0x0F) == 0 ? FH : 0) | (v == 0x7F ? FP : 0));
    return r;
}

static uint8_t dec8(uint8_t v)
{
    uint8_t r = (uint8_t)(v - 1);
    Z.f = (uint8_t)((Z.f & FC) | (r & (FS | FY | FX)) | (r ? 0 : FZ) |
          ((v & 0x0F) == 0 ? FH : 0) | (v == 0x80 ? FP : 0) | FN);
    return r;
}

static uint16_t add16(uint16_t a, uint16_t b)
{
    unsigned r = (unsigned)a + b;
    Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | ((r >> 8) & (FY | FX)) |
          (((a ^ b ^ r) & 0x1000) ? FH : 0) | (r > 0xFFFF ? FC : 0));
    return (uint16_t)r;
}

static uint16_t adc16(uint16_t a, uint16_t b)
{
    unsigned c = Z.f & FC;
    unsigned r = (unsigned)a + b + c;
    uint16_t res = (uint16_t)r;
    Z.f = (uint8_t)(((res >> 8) & (FS | FY | FX)) | (res ? 0 : FZ) |
          (((a ^ b ^ r) & 0x1000) ? FH : 0) |
          ((~(a ^ b) & (a ^ res) & 0x8000) ? FP : 0) | (r > 0xFFFF ? FC : 0));
    return res;
}

static uint16_t sbc16(uint16_t a, uint16_t b)
{
    unsigned c = Z.f & FC;
    unsigned r = (unsigned)a - b - c;
    uint16_t res = (uint16_t)r;
    Z.f = (uint8_t)(((res >> 8) & (FS | FY | FX)) | (res ? 0 : FZ) |
          (((a ^ b ^ r) & 0x1000) ? FH : 0) |
          (((a ^ b) & (a ^ res) & 0x8000) ? FP : 0) | FN |
          (r > 0xFFFF ? FC : 0));
    return res;
}

static int cond(int cc)
{
    switch (cc) {
    case 0: return !(Z.f & FZ);
    case 1: return Z.f & FZ;
    case 2: return !(Z.f & FC);
    case 3: return Z.f & FC;
    case 4: return !(Z.f & FP);
    case 5: return Z.f & FP;
    case 6: return !(Z.f & FS);
    default: return Z.f & FS;
    }
}

static void daa(void)
{
    uint8_t a = Z.a, adj = 0, cy = Z.f & FC;
    if ((Z.f & FH) || (a & 0x0F) > 9)
        adj |= 0x06;
    if (cy || a > 0x99) {
        adj |= 0x60;
        cy = FC;
    }
    if (Z.f & FN) {
        uint8_t h = ((Z.f & FH) && (a & 0x0F) < 6) ? FH : 0;
        Z.a = (uint8_t)(a - adj);
        Z.f = (uint8_t)((Z.a & (FS | FY | FX)) | (Z.a ? 0 : FZ) | h |
                        par(Z.a) | FN | cy);
    } else {
        uint8_t h = (a & 0x0F) > 9 ? FH : 0;
        Z.a = (uint8_t)(a + adj);
        Z.f = (uint8_t)((Z.a & (FS | FY | FX)) | (Z.a ? 0 : FZ) | h |
                        par(Z.a) | cy);
    }
}

/* rotate/shift group (CB 00-3F): y = rlc rrc rl rr sla sra sll srl */
static uint8_t rot(int y, uint8_t v)
{
    uint8_t c, r;
    switch (y) {
    case 0: c = v >> 7; r = (uint8_t)((v << 1) | c); break;
    case 1: c = v & 1; r = (uint8_t)((v >> 1) | (c << 7)); break;
    case 2: c = v >> 7; r = (uint8_t)((v << 1) | (Z.f & FC)); break;
    case 3: c = v & 1; r = (uint8_t)((v >> 1) | ((Z.f & FC) << 7)); break;
    case 4: c = v >> 7; r = (uint8_t)(v << 1); break;
    case 5: c = v & 1; r = (uint8_t)((v >> 1) | (v & 0x80)); break;
    case 6: c = v >> 7; r = (uint8_t)((v << 1) | 1); break;
    default: c = v & 1; r = (uint8_t)(v >> 1); break;
    }
    Z.f = (uint8_t)((r & (FS | FY | FX)) | (r ? 0 : FZ) | par(r) | c);
    return r;
}

static void do_cb(uint16_t addr, int indexed)
{
    uint8_t op = fetch8();
    int x = op >> 6, y = (op >> 3) & 7, zr = op & 7;
    uint8_t v;
    int mem_op = indexed || zr == 6;

    v = mem_op ? rb(addr) : get_r(zr, 0);
    switch (x) {
    case 0:
        v = rot(y, v);
        break;
    case 1: {
        uint8_t bit = (uint8_t)(v & (1 << y));
        Z.f = (uint8_t)((Z.f & FC) | FH | (bit ? 0 : (FZ | FP)) |
                        (bit & FS) | (v & (FY | FX)));
        return;
    }
    case 2:
        v &= (uint8_t)~(1 << y);
        break;
    default:
        v |= (uint8_t)(1 << y);
        break;
    }
    if (mem_op)
        wb(addr, v);
    if (!mem_op || (indexed && zr != 6))
        set_r(zr, 0, v);
}

static void block_op(int y, int zr)
{
    uint16_t hl = get_hl(), de = get_de(), bc = get_bc();
    int dir = (y & 1) ? -1 : 1;           /* ldd/cpd/... go downward */
    int repeat = y >= 6;
    (void)dir;
    if (zr == 0) {                        /* ldi ldd ldir lddr */
        for (;;) {
            wb(de, rb(hl));
            hl = (uint16_t)(hl + (y & 1 ? -1 : 1));
            de = (uint16_t)(de + (y & 1 ? -1 : 1));
            bc--;
            if (!repeat || bc == 0)
                break;
        }
        set_hl(hl); set_de(de); set_bc(bc);
        Z.f = (uint8_t)((Z.f & (FS | FZ | FC)) | (bc ? FP : 0));
    } else if (zr == 1) {                 /* cpi cpd cpir cpdr */
        uint8_t saved_c = Z.f & FC, r = 0;
        for (;;) {
            r = sub8(Z.a, rb(hl), 0);
            hl = (uint16_t)(hl + (y & 1 ? -1 : 1));
            bc--;
            if (!repeat || bc == 0 || r == 0)
                break;
        }
        set_hl(hl); set_bc(bc);
        Z.f = (uint8_t)((Z.f & (FS | FZ | FH | FN)) | saved_c |
                        (bc ? FP : 0));
    } else if (zr == 2) {                 /* ini ind inir indr */
        for (;;) {
            wb(hl, z80_in(Z.c));
            hl = (uint16_t)(hl + (y & 1 ? -1 : 1));
            Z.b--;
            if (!repeat || Z.b == 0)
                break;
        }
        set_hl(hl);
        Z.f = (uint8_t)((Z.b ? 0 : FZ) | FN);
    } else {                              /* outi outd otir otdr */
        for (;;) {
            z80_out(Z.c, rb(hl));
            hl = (uint16_t)(hl + (y & 1 ? -1 : 1));
            Z.b--;
            if (!repeat || Z.b == 0)
                break;
        }
        set_hl(hl);
        Z.f = (uint8_t)((Z.b ? 0 : FZ) | FN);
    }
}

static void do_ed(void)
{
    uint8_t op = fetch8();
    int x = op >> 6, y = (op >> 3) & 7, zr = op & 7, p = y >> 1, q = y & 1;
    uint16_t nn;

    if (x == 2) {
        if (y >= 4 && zr < 4)
            block_op(y, zr);
        return;
    }
    if (x != 1)
        return;                           /* ED NOPs */
    switch (zr) {
    case 0: {                             /* in r,(c) */
        uint8_t v = z80_in(Z.c);
        if (y != 6)
            set_r(y, 0, v);
        Z.f = (uint8_t)((Z.f & FC) | (v & (FS | FY | FX)) | (v ? 0 : FZ) |
                        par(v));
        break;
    }
    case 1:                               /* out (c),r */
        z80_out(Z.c, y == 6 ? 0 : get_r(y, 0));
        break;
    case 2:                               /* sbc/adc hl,rp */
        if (q == 0)
            set_hl(sbc16(get_hl(), get_rp(p, 0)));
        else
            set_hl(adc16(get_hl(), get_rp(p, 0)));
        break;
    case 3:                               /* ld (nn),rp / ld rp,(nn) */
        nn = fetch16();
        if (q == 0)
            ww(nn, get_rp(p, 0));
        else
            set_rp(p, 0, rw(nn));
        break;
    case 4: {                             /* neg */
        uint8_t a = Z.a;
        Z.a = sub8(0, a, 0);
        break;
    }
    case 5:                               /* retn/reti */
        Z.pc = pop16();
        Z.iff1 = Z.iff2;
        break;
    case 6:                               /* im */
        Z.im = (y & 3) == 0 ? 0 : ((y & 3) == 2 ? 1 : 2);
        break;
    default:
        switch (y) {
        case 0: Z.i = Z.a; break;
        case 1: Z.r = Z.a; break;
        case 2: case 3: {
            uint8_t v = y == 2 ? Z.i : Z.r;
            Z.a = v;
            Z.f = (uint8_t)((Z.f & FC) | (v & (FS | FY | FX)) | (v ? 0 : FZ) |
                            (Z.iff2 ? FP : 0));
            break;
        }
        case 4: {                         /* rrd */
            uint8_t m = rb(get_hl()), a = Z.a;
            wb(get_hl(), (uint8_t)((a << 4) | (m >> 4)));
            Z.a = (uint8_t)((a & 0xF0) | (m & 0x0F));
            Z.f = (uint8_t)((Z.f & FC) | (Z.a & (FS | FY | FX)) |
                            (Z.a ? 0 : FZ) | par(Z.a));
            break;
        }
        case 5: {                         /* rld */
            uint8_t m = rb(get_hl()), a = Z.a;
            wb(get_hl(), (uint8_t)((m << 4) | (a & 0x0F)));
            Z.a = (uint8_t)((a & 0xF0) | (m >> 4));
            Z.f = (uint8_t)((Z.f & FC) | (Z.a & (FS | FY | FX)) |
                            (Z.a ? 0 : FZ) | par(Z.a));
            break;
        }
        default: break;
        }
        break;
    }
}

/* Decode one instruction.  idx is NULL, &Z.ix or &Z.iy (DD/FD prefix). */
static void exec(uint16_t *idx)
{
    uint8_t op = fetch8();
    int x = op >> 6, y = (op >> 3) & 7, zr = op & 7, p = y >> 1, q = y & 1;
    uint16_t addr = 0, nn;
    int8_t d;

    Z.r = (uint8_t)((Z.r & 0x80) | ((Z.r + 1) & 0x7F));

    if (idx && (op == 0xCB)) {            /* DD CB d op */
        d = (int8_t)fetch8();
        do_cb((uint16_t)(*idx + d), 1);
        return;
    }
    /* does this opcode address (HL)? then use (IX+d) under a prefix */
    if (idx && ((x == 1 && (y == 6 || zr == 6) && op != 0x76) ||
                (x == 2 && zr == 6) ||
                (x == 0 && (zr == 4 || zr == 5 || zr == 6) && y == 6))) {
        d = (int8_t)fetch8();
        addr = (uint16_t)(*idx + d);
    } else {
        addr = get_hl();
    }

    switch (x) {
    case 1:
        if (op == 0x76) {
            Z.halted = 1;
            Z.pc--;
            return;
        }
        if (idx && y == 6) {              /* ld (ix+d),r : r is H/L plain */
            wb(addr, get_r(zr, 0));
        } else if (idx && zr == 6) {      /* ld r,(ix+d) */
            set_r(y, 0, rb(addr));
        } else {
            set_r(y, addr, get_r(zr, addr));
        }
        return;
    case 2:
        alu(y, get_r(zr, addr));
        return;
    case 0:
        switch (zr) {
        case 0:
            switch (y) {
            case 0: return;
            case 1: {
                uint8_t t;
                t = Z.a; Z.a = Z.a2; Z.a2 = t;
                t = Z.f; Z.f = Z.f2; Z.f2 = t;
                return;
            }
            case 2:                       /* djnz */
                d = (int8_t)fetch8();
                if (--Z.b)
                    Z.pc = (uint16_t)(Z.pc + d);
                return;
            case 3:                       /* jr */
                d = (int8_t)fetch8();
                Z.pc = (uint16_t)(Z.pc + d);
                return;
            default:                      /* jr cc */
                d = (int8_t)fetch8();
                if (cond(y - 4))
                    Z.pc = (uint16_t)(Z.pc + d);
                return;
            }
        case 1:
            if (q == 0)
                set_rp(p, idx, fetch16());
            else if (idx && p == 2)
                *idx = add16(*idx, *idx);
            else if (idx)
                *idx = add16(*idx, get_rp(p, idx));
            else
                set_hl(add16(get_hl(), get_rp(p, 0)));
            return;
        case 2:
            switch (y) {
            case 0: wb(get_bc(), Z.a); return;
            case 1: Z.a = rb(get_bc()); return;
            case 2: wb(get_de(), Z.a); return;
            case 3: Z.a = rb(get_de()); return;
            case 4: ww(fetch16(), idx ? *idx : get_hl()); return;
            case 5: set_rp(2, idx, rw(fetch16())); return;
            case 6: wb(fetch16(), Z.a); return;
            default: Z.a = rb(fetch16()); return;
            }
        case 3:
            set_rp(p, idx, (uint16_t)(get_rp(p, idx) + (q ? -1 : 1)));
            return;
        case 4:
            set_r(y, addr, inc8(get_r(y, addr)));
            return;
        case 5:
            set_r(y, addr, dec8(get_r(y, addr)));
            return;
        case 6:
            set_r(y, addr, fetch8());
            return;
        default:
            switch (y) {
            case 0: {                     /* rlca */
                uint8_t c = Z.a >> 7;
                Z.a = (uint8_t)((Z.a << 1) | c);
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) | c);
                return;
            }
            case 1: {                     /* rrca */
                uint8_t c = Z.a & 1;
                Z.a = (uint8_t)((Z.a >> 1) | (c << 7));
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) | c);
                return;
            }
            case 2: {                     /* rla */
                uint8_t c = Z.a >> 7;
                Z.a = (uint8_t)((Z.a << 1) | (Z.f & FC));
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) | c);
                return;
            }
            case 3: {                     /* rra */
                uint8_t c = Z.a & 1;
                Z.a = (uint8_t)((Z.a >> 1) | ((Z.f & FC) << 7));
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) | c);
                return;
            }
            case 4: daa(); return;
            case 5:                       /* cpl */
                Z.a = (uint8_t)~Z.a;
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP | FC)) | FH | FN |
                                (Z.a & (FY | FX)));
                return;
            case 6:                       /* scf */
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) | FC);
                return;
            default:                      /* ccf */
                Z.f = (uint8_t)((Z.f & (FS | FZ | FP)) | (Z.a & (FY | FX)) |
                                ((Z.f & FC) ? FH : FC));
                return;
            }
        }
    default:
        break;
    }

    /* x == 3 */
    switch (zr) {
    case 0:
        if (cond(y))
            Z.pc = pop16();
        return;
    case 1:
        if (q == 0) {
            nn = pop16();
            switch (p) {
            case 0: set_bc(nn); break;
            case 1: set_de(nn); break;
            case 2: if (idx) *idx = nn; else set_hl(nn); break;
            default: Z.a = nn >> 8; Z.f = (uint8_t)nn; break;
            }
            return;
        }
        switch (p) {
        case 0: Z.pc = pop16(); return;
        case 1: {                         /* exx */
            uint8_t t;
            t = Z.b; Z.b = Z.b2; Z.b2 = t;
            t = Z.c; Z.c = Z.c2; Z.c2 = t;
            t = Z.d; Z.d = Z.d2; Z.d2 = t;
            t = Z.e; Z.e = Z.e2; Z.e2 = t;
            t = Z.h; Z.h = Z.h2; Z.h2 = t;
            t = Z.l; Z.l = Z.l2; Z.l2 = t;
            return;
        }
        case 2: Z.pc = idx ? *idx : get_hl(); return;
        default: Z.sp = idx ? *idx : get_hl(); return;
        }
    case 2:
        nn = fetch16();
        if (cond(y))
            Z.pc = nn;
        return;
    case 3:
        switch (y) {
        case 0: Z.pc = fetch16(); return;
        case 1: do_cb(get_hl(), 0); return;
        case 2: z80_out(fetch8(), Z.a); return;
        case 3: Z.a = z80_in(fetch8()); return;
        case 4:                           /* ex (sp),hl */
            nn = rw(Z.sp);
            ww(Z.sp, idx ? *idx : get_hl());
            if (idx) *idx = nn; else set_hl(nn);
            return;
        case 5: {                         /* ex de,hl */
            uint16_t t = get_de();
            set_de(get_hl());
            set_hl(t);
            return;
        }
        case 6: Z.iff1 = Z.iff2 = 0; return;
        default: Z.iff1 = Z.iff2 = 1; return;
        }
    case 4:
        nn = fetch16();
        if (cond(y)) {
            push16(Z.pc);
            Z.pc = nn;
        }
        return;
    case 5:
        if (q == 0) {
            switch (p) {
            case 0: push16(get_bc()); break;
            case 1: push16(get_de()); break;
            case 2: push16(idx ? *idx : get_hl()); break;
            default: push16((uint16_t)((Z.a << 8) | Z.f)); break;
            }
            return;
        }
        switch (p) {
        case 0:
            nn = fetch16();
            push16(Z.pc);
            Z.pc = nn;
            return;
        case 1: exec(&Z.ix); return;
        case 2: do_ed(); return;
        default: exec(&Z.iy); return;
        }
    case 6:
        alu(y, fetch8());
        return;
    default:
        push16(Z.pc);
        Z.pc = (uint16_t)(y * 8);
        return;
    }
}

void z80_step(void)
{
    exec(0);
}
