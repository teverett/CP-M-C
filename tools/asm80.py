#!/usr/bin/env python3
"""Minimal 8080 assembler, just enough for orig/os/bdos.asm, orig/os/ccp.asm and
the test programs.  Mimics the subset of Macro Assembler AS used there.

usage: asm80.py [-D name=value]... -o out.bin [-s out.sym] file.asm
The binary covers org..highest emitted address (like p2bin -r).
"""
import re
import sys

REG = {'b': 0, 'c': 1, 'd': 2, 'e': 3, 'h': 4, 'l': 5, 'm': 6, 'a': 7}
RP = {'b': 0, 'd': 1, 'h': 2, 'sp': 3}
RPP = {'b': 0, 'd': 1, 'h': 2, 'psw': 3}

NOARG = dict(nop=0x00, hlt=0x76, ret=0xC9, rnz=0xC0, rz=0xC8, rnc=0xD0,
             rc=0xD8, rpo=0xE0, rpe=0xE8, rp=0xF0, rm=0xF8, xchg=0xEB,
             xthl=0xE3, sphl=0xF9, pchl=0xE9, ei=0xFB, di=0xF3, daa=0x27,
             cma=0x2F, stc=0x37, cmc=0x3F, rlc=0x07, rrc=0x0F, ral=0x17,
             rar=0x1F)
ALU = dict(add=0x80, adc=0x88, sub=0x90, sbb=0x98, ana=0xA0, xra=0xA8,
           ora=0xB0, cmp=0xB8)
ALUI = dict(adi=0xC6, aci=0xCE, sui=0xD6, sbi=0xDE, ani=0xE6, xri=0xEE,
            ori=0xF6, cpi=0xFE)
ADDR = dict(jmp=0xC3, jnz=0xC2, jz=0xCA, jnc=0xD2, jc=0xDA, jpo=0xE2,
            jpe=0xEA, jp=0xF2, jm=0xFA, call=0xCD, cnz=0xC4, cz=0xCC,
            cnc=0xD4, cc=0xDC, cpo=0xE4, cpe=0xEC, cp=0xF4, cm=0xFC,
            lda=0x3A, sta=0x32, lhld=0x2A, shld=0x22)

TOKEN = re.compile(r"""\s*(?:
    (?P<chr>'[^']'|"[^"]")|
    (?P<num>[0-9][0-9A-Za-z]*)|
    (?P<sym>[A-Za-z_.][A-Za-z_0-9.]*)|
    (?P<dollar>\$)|
    (?P<op>[-+*/&|~()^]|<<|>>))""", re.X)


class Asm:
    def __init__(self, defs):
        self.defs = dict(defs)
        self.sym = {}
        self.known = {}

    def num(self, t):
        tl = t.lower()
        if tl.endswith('h') and re.fullmatch(r'[0-9][0-9a-f]*h', tl):
            return int(tl[:-1], 16)
        if tl.endswith('b') and re.fullmatch(r'[01]+b', tl):
            return int(tl[:-1], 2)
        if tl.isdigit():
            return int(tl)
        raise ValueError('bad number ' + t)

    def expr(self, s, pc):
        out = []
        pos = 0
        s = s.strip()
        while pos < len(s):
            m = TOKEN.match(s, pos)
            if not m or m.end() == pos:
                raise ValueError('bad expression: ' + s)
            pos = m.end()
            if m.group('chr'):
                out.append(str(ord(m.group('chr')[1])))
            elif m.group('num'):
                out.append(str(self.num(m.group('num'))))
            elif m.group('sym'):
                name = m.group('sym')
                if name in self.sym:
                    out.append(str(self.sym[name]))
                elif name in self.known:
                    out.append(str(self.known[name]))
                else:
                    self.missing = True
                    out.append('0')
            elif m.group('dollar'):
                out.append(str(pc))
            else:
                op = m.group('op')
                out.append('//' if op == '/' else op)
        return eval(''.join(out), {}, {})

    @staticmethod
    def split_args(s):
        args, cur, q = [], '', None
        for ch in s:
            if q:
                cur += ch
                if ch == q:
                    q = None
            elif ch in '\'"':
                q = ch
                cur += ch
            elif ch == ',':
                args.append(cur.strip())
                cur = ''
            else:
                cur += ch
        if cur.strip():
            args.append(cur.strip())
        return args

    @staticmethod
    def strip_comment(line):
        q = None
        for i, ch in enumerate(line):
            if q:
                if ch == q:
                    q = None
            elif ch in '\'"':
                q = ch
            elif ch == ';':
                return line[:i]
        return line

    def assemble(self, lines):
        self.known = dict(self.sym)
        self.sym = dict(self.defs)
        self.missing = False
        mem = {}
        pc = 0
        org = None
        stack = []          # (active, taken)
        active = True

        def emit(*bs):
            nonlocal pc
            for b in bs:
                mem[pc] = b & 0xFF
                pc += 1

        for raw in lines:
            line = self.strip_comment(raw).rstrip()
            if not line.strip():
                continue
            m = re.match(r'^([A-Za-z_][A-Za-z_0-9]*)\s*:\s*(.*)$', line)
            label = None
            rest = line
            if m:
                label, rest = m.group(1), m.group(2)
            else:
                m = re.match(r'^([A-Za-z_][A-Za-z_0-9]*)\s+(equ|set)\s+(.*)$',
                             line, re.I)
                if m and active:
                    v = self.expr(m.group(3), pc)
                    self.sym[m.group(1)] = v
                    continue
            rest = rest.strip()
            mm = re.match(r'^(\.?[A-Za-z]+)\s*(.*)$', rest)
            op = mm.group(1).lower() if mm else ''
            arg = mm.group(2).strip() if mm else ''
            # conditionals
            if op in ('ifdef', 'ifndef', 'if'):
                if active:
                    if op == 'if':
                        c = self.expr(arg, pc) != 0
                    else:
                        c = (arg in self.sym) == (op == 'ifdef')
                else:
                    c = False
                stack.append((active, c))
                active = active and c
                continue
            if op == 'else':
                outer, c = stack[-1]
                active = outer and not c
                continue
            if op == 'endif':
                active, _ = stack.pop()
                continue
            if not active:
                continue
            if label:
                self.sym[label] = pc
            if not op:
                continue
            if op in ('.cpu', 'title', 'end'):
                continue
            if op == 'org':
                pc = self.expr(arg, pc)
                if org is None:
                    org = pc
                continue
            if op == 'db':
                for a in self.split_args(arg):
                    if a[0] in '\'"' and len(a) > 3 and a[-1] == a[0]:
                        emit(*[ord(ch) for ch in a[1:-1]])
                    else:
                        emit(self.expr(a, pc))
                continue
            if op == 'dw':
                for a in self.split_args(arg):
                    v = self.expr(a, pc)
                    emit(v, v >> 8)
                continue
            if op == 'ds':
                n = self.expr(arg, pc)
                for _ in range(n):
                    emit(0)
                continue
            a = [x.strip() for x in self.split_args(arg)]
            if op in NOARG:
                emit(NOARG[op])
            elif op == 'mov':
                emit(0x40 | REG[a[0].lower()] << 3 | REG[a[1].lower()])
            elif op == 'mvi':
                emit(0x06 | REG[a[0].lower()] << 3, self.expr(a[1], pc))
            elif op == 'lxi':
                v = self.expr(a[1], pc)
                emit(0x01 | RP[a[0].lower()] << 4, v, v >> 8)
            elif op in ('ldax', 'stax'):
                emit((0x0A if op == 'ldax' else 0x02) | RP[a[0].lower()] << 4)
            elif op in ('inx', 'dcx', 'dad'):
                base = dict(inx=0x03, dcx=0x0B, dad=0x09)[op]
                emit(base | RP[a[0].lower()] << 4)
            elif op in ('inr', 'dcr'):
                emit((0x04 if op == 'inr' else 0x05) | REG[a[0].lower()] << 3)
            elif op in ('push', 'pop'):
                emit((0xC5 if op == 'push' else 0xC1) |
                     RPP[a[0].lower()] << 4)
            elif op in ALU:
                emit(ALU[op] | REG[a[0].lower()])
            elif op in ALUI:
                emit(ALUI[op], self.expr(a[0], pc))
            elif op in ADDR:
                v = self.expr(a[0], pc)
                emit(ADDR[op], v, v >> 8)
            elif op == 'rst':
                emit(0xC7 | self.expr(a[0], pc) << 3)
            elif op in ('in', 'out'):
                emit(0xDB if op == 'in' else 0xD3, self.expr(a[0], pc))
            else:
                raise ValueError('unknown op %r in %r' % (op, raw))
        return org, mem

    def run(self, lines):
        for _ in range(8):
            org, mem = self.assemble(lines)
            if not self.missing and self.sym == self.known:
                break
        else:
            raise ValueError('did not converge (undefined symbol?)')
        lo, hi = min(mem), max(mem)
        return org, bytes(mem.get(i, 0) for i in range(lo, hi + 1))


def main(argv):
    defs, out, sym, src = {}, None, None, None
    i = 1
    while i < len(argv):
        if argv[i] == '-D':
            k, v = argv[i + 1].split('=', 1)
            defs[k] = Asm({}).expr(v, 0)
            i += 2
        elif argv[i] == '-o':
            out = argv[i + 1]
            i += 2
        elif argv[i] == '-s':
            sym = argv[i + 1]
            i += 2
        else:
            src = argv[i]
            i += 1
    a = Asm(defs)
    with open(src) as f:
        org, data = a.run(f.read().splitlines())
    with open(out, 'wb') as f:
        f.write(data)
    if sym:
        with open(sym, 'w') as f:
            for k, v in sorted(a.sym.items(), key=lambda kv: (kv[1], kv[0])):
                f.write('%-16s %04X\n' % (k, v & 0xFFFF))
    print('%s: org %04X, %d bytes' % (src, org, len(data)))


if __name__ == '__main__':
    main(sys.argv)
