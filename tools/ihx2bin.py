#!/usr/bin/env python3
"""Convert Intel HEX to a flat binary starting at address 0, zero filled."""
import sys

mem = {}
for line in open(sys.argv[1]):
    line = line.strip()
    if not line.startswith(':'):
        continue
    n = int(line[1:3], 16)
    addr = int(line[3:7], 16)
    typ = int(line[7:9], 16)
    if typ == 0:
        for i in range(n):
            mem[addr + i] = int(line[9 + 2 * i:11 + 2 * i], 16)
size = max(mem) + 1
out = bytearray(size)
for a, v in mem.items():
    out[a] = v
open(sys.argv[2], 'wb').write(out)
