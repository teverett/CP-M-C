#!/usr/bin/env python3
"""Convert Intel HEX records into a CP/M COM image based at 0100h."""
import sys

BASE = 0x0100
memory = {}
for line in open(sys.argv[1], encoding='ascii'):
    line = line.strip()
    if not line.startswith(':'):
        continue
    count = int(line[1:3], 16)
    address = int(line[3:7], 16)
    record_type = int(line[7:9], 16)
    if record_type == 0:
        for offset in range(count):
            memory[address + offset] = int(line[9 + 2 * offset:11 + 2 * offset], 16)

if not memory or min(memory) < BASE:
    raise SystemExit('Intel HEX image has no valid COM data at or above 0100h')

image = bytearray(max(memory) - BASE + 1)
for address, value in memory.items():
    image[address - BASE] = value
with open(sys.argv[2], 'wb') as output:
    output.write(image)
