#!/usr/bin/env python3
"""Disassemble reachable 8080 code in a flat binary, retaining data as DB."""
import argparse
from collections import deque
from pathlib import Path


REG = ("b", "c", "d", "e", "h", "l", "m", "a")
RP = ("b", "d", "h", "sp")
RPP = ("b", "d", "h", "psw")
COND = ("nz", "z", "nc", "c", "po", "pe", "p", "m")
ALU = ("add", "adc", "sub", "sbb", "ana", "xra", "ora", "cmp")
ALUI = ("adi", "aci", "sui", "sbi", "ani", "xri", "ori", "cpi")


def literal(value, width):
    digits = f"{value:0{width}X}"
    if digits[0] in "ABCDEF":
        digits = "0" + digits
    return digits + "h"


def hx(value):
    return literal(value & 0xFFFF, 4)


def data_tokens(values):
    parts = []
    index = 0
    while index < len(values):
        end = index
        while end < len(values) and 0x20 <= values[end] <= 0x7E and values[end] not in (0x22, 0x27):
            end += 1
        if end - index >= 4:
            parts.append((f'"{bytes(values[index:end]).decode("ascii")}"', end - index))
            index = end
            continue
        if end > index:
            parts.extend((literal(value, 2), 1) for value in values[index:end])
            index = end
            continue
        parts.append((literal(values[index], 2), 1))
        index += 1
    return parts


def decode(data, offset, base):
    """Return mnemonic, size, control-flow targets, and whether flow continues."""
    op = data[offset]
    pc = base + offset
    remaining = len(data) - offset

    def byte(index):
        return data[offset + index] if remaining > index else 0

    def word():
        return byte(1) | byte(2) << 8

    def addr(size, targets=(), continues=True):
        return size, tuple(targets), continues

    text = None
    if 0x40 <= op <= 0x7F:
        text = "hlt" if op == 0x76 else f"mov {REG[(op >> 3) & 7]}, {REG[op & 7]}"
        size, targets, continues = addr(1, continues=op != 0x76)
    elif 0x80 <= op <= 0xBF:
        text = f"{ALU[(op >> 3) & 7]} {REG[op & 7]}"
        size, targets, continues = addr(1)
    elif op in (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38):
        text = "nop"
        size, targets, continues = addr(1)
    elif op in (0x07, 0x0F, 0x17, 0x1F):
        text = {0x07: "rlc", 0x0F: "rrc", 0x17: "ral", 0x1F: "rar"}[op]
        size, targets, continues = addr(1)
    elif op == 0x27 or op == 0x2F or op == 0x37 or op == 0x3F:
        text = {0x27: "daa", 0x2F: "cma", 0x37: "stc", 0x3F: "cmc"}[op]
        size, targets, continues = addr(1)
    elif op & 0xCF == 0x01:
        text = f"lxi {RP[(op >> 4) & 3]}, {hx(word())}"
        size, targets, continues = addr(3)
    elif op in (0x02, 0x12):
        text = f"stax {RP[(op >> 4) & 1]}"
        size, targets, continues = addr(1)
    elif op in (0x0A, 0x1A):
        text = f"ldax {RP[(op >> 4) & 1]}"
        size, targets, continues = addr(1)
    elif op & 0xCF == 0x03:
        text = f"inx {RP[(op >> 4) & 3]}"
        size, targets, continues = addr(1)
    elif op & 0xCF == 0x09:
        text = f"dad {RP[(op >> 4) & 3]}"
        size, targets, continues = addr(1)
    elif op & 0xCF == 0x0B:
        text = f"dcx {RP[(op >> 4) & 3]}"
        size, targets, continues = addr(1)
    elif op & 0xC7 == 0x04:
        text = f"inr {REG[(op >> 3) & 7]}"
        size, targets, continues = addr(1)
    elif op & 0xC7 == 0x05:
        text = f"dcr {REG[(op >> 3) & 7]}"
        size, targets, continues = addr(1)
    elif op & 0xC7 == 0x06:
        text = f"mvi {REG[(op >> 3) & 7]}, {literal(byte(1), 2)}"
        size, targets, continues = addr(2)
    elif op in (0x22, 0x2A, 0x32, 0x3A):
        text = f"{ {0x22: 'shld', 0x2A: 'lhld', 0x32: 'sta', 0x3A: 'lda'}[op] } {hx(word())}"
        size, targets, continues = addr(3)
    elif op in (0xC3, 0xCB):
        text = f"jmp {hx(word())}"
        size, targets, continues = addr(3, (word(),), False)
    elif op in (0xC9, 0xD9):
        text = "ret"
        size, targets, continues = addr(1, continues=False)
    elif op in (0xCD, 0xDD, 0xED, 0xFD):
        text = f"call {hx(word())}"
        size, targets, continues = addr(3, (word(),))
    elif op == 0xE9:
        text = "pchl"
        size, targets, continues = addr(1, continues=False)
    elif op == 0xF9:
        text = "sphl"
        size, targets, continues = addr(1)
    elif op == 0xE3:
        text = "xthl"
        size, targets, continues = addr(1)
    elif op == 0xEB:
        text = "xchg"
        size, targets, continues = addr(1)
    elif op == 0xF3 or op == 0xFB:
        text = "di" if op == 0xF3 else "ei"
        size, targets, continues = addr(1)
    elif op in (0xD3, 0xDB):
        text = f"{'out' if op == 0xD3 else 'in'} {literal(byte(1), 2)}"
        size, targets, continues = addr(2)
    elif op & 0xCF == 0xC1:
        index = (op >> 4) & 3
        text = f"pop {RPP[index]}"
        size, targets, continues = addr(1)
    elif op & 0xCF == 0xC5:
        text = f"push {RPP[(op >> 4) & 3]}"
        size, targets, continues = addr(1)
    elif op & 0xC7 == 0xC0:
        text = f"r{COND[(op >> 3) & 7]}"
        size, targets, continues = addr(1)
    elif op & 0xC7 == 0xC2:
        target = word()
        text = f"j{COND[(op >> 3) & 7]} {hx(target)}"
        size, targets, continues = addr(3, (target,))
    elif op & 0xC7 == 0xC4:
        target = word()
        text = f"c{COND[(op >> 3) & 7]} {hx(target)}"
        size, targets, continues = addr(3, (target,))
    elif op & 0xC7 == 0xC6:
        text = f"{ALUI[(op >> 3) & 7]} {literal(byte(1), 2)}"
        size, targets, continues = addr(2)
    elif op & 0xC7 == 0xC7:
        text = f"rst {(op >> 3) & 7}"
        size, targets, continues = addr(1, ((op >> 3) & 7) * 8)
    else:
        text = f"db {literal(op, 2)}"
        size, targets, continues = addr(1)

    size = min(size, remaining)
    next_pc = pc + size
    in_image = lambda target: base <= target < base + len(data)
    targets = tuple(target for target in targets if in_image(target))
    if continues and next_pc < base + len(data):
        targets += (next_pc,)
    return text, size, targets


def disassemble(data, base):
    pending = deque([base])
    instructions = {}
    occupied = {}
    while pending:
        pc = pending.popleft()
        if not base <= pc < base + len(data) or pc in instructions:
            continue
        offset = pc - base
        if offset in occupied:
            continue
        text, size, successors = decode(data, offset, base)
        if any(pos in occupied for pos in range(offset, offset + size)):
            continue
        instructions[pc] = (text, size)
        for pos in range(offset, offset + size):
            occupied[pos] = pc
        pending.extend(successors)

    lines = [
        "; Control-flow disassembly of MAC.COM for Intel 8080.",
        "; Reachable instructions are decoded; unreachable bytes remain DB data.",
        "; Reassemble from the repository root with tools/asm80.py.",
        ".cpu 8080",
        f"org {base:04X}h",
    ]
    offset = 0
    while offset < len(data):
        pc = base + offset
        if pc in instructions:
            text, size = instructions[pc]
            chunk = data[offset:offset + size]
            raw = " ".join(f"{byte:02X}" for byte in chunk)
            alias_comment = ""
            op = chunk[0]
            if op in (0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38):
                text = f"db 0{op:02X}h"
                alias_comment = " ; nop alias"
            elif op == 0xCB:
                text = f"db {literal(op, 2)}, {literal(chunk[1], 2)}, {literal(chunk[2], 2)}"
                alias_comment = f" ; jmp {hx(chunk[1] | chunk[2] << 8)} alias"
            elif op == 0xD9:
                text = "db 0D9h"
                alias_comment = " ; ret alias"
            elif op in (0xDD, 0xED, 0xFD):
                text = f"db {literal(op, 2)}, {literal(chunk[1], 2)}, {literal(chunk[2], 2)}"
                alias_comment = f" ; call {hx(chunk[1] | chunk[2] << 8)} alias"
            lines.append(f"{text:<30} ; {pc:04X}: {raw}{alias_comment}")
            offset += size
        else:
            run = []
            while offset < len(data) and base + offset not in instructions:
                run.append(data[offset])
                offset += 1
            current = []
            current_bytes = 0
            for token, count in data_tokens(run):
                proposed = ", ".join(current + [token])
                if current and len("db " + proposed) > 72:
                    values = ", ".join(current)
                    lines.append(f"{'db ' + values:<30} ; {pc:04X}: data")
                    pc += current_bytes
                    current = []
                    current_bytes = 0
                current.append(token)
                current_bytes += count
            if current:
                values = ", ".join(current)
                lines.append(f"{'db ' + values:<30} ; {pc:04X}: data")
    return "\n".join(lines) + "\n", len(instructions)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("-o", "--output", type=Path)
    parser.add_argument("-b", "--base", type=lambda n: int(n, 0), default=0x100)
    args = parser.parse_args()
    source, count = disassemble(args.input.read_bytes(), args.base)
    if args.output:
        args.output.write_text(source, encoding="ascii")
    else:
        print(source, end="")
    print(f"Disassembled {count} reachable instruction(s).", file=__import__("sys").stderr)


if __name__ == "__main__":
    main()
