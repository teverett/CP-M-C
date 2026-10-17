#!/usr/bin/env python3
"""Build a CP/M disk image for the test host (see src/host.c geometries).

usage: mkdisk.py TYPE out.img [USER:]NAME.EXT[;ro][;sys]=hostfile ...
   TYPE 0 = 8" IBM 3740 (drive A), 1 = 2K blocks/16-bit map (B),
   2 = 4K blocks/EXM=3 (C).  Files are laid out contiguously.
"""
import sys

SKEW6 = [1, 7, 13, 19, 25, 5, 11, 17, 23, 3, 9, 15, 21,
         2, 8, 14, 20, 26, 6, 12, 18, 24, 4, 10, 16, 22]

GEOMS = [
    dict(spt=26, bsh=3, exm=0, dsm=242, drm=63, al0=0xC0, al1=0, off=2,
         tracks=77, xlt=SKEW6),
    dict(spt=64, bsh=4, exm=0, dsm=299, drm=127, al0=0xC0, al1=0, off=1,
         tracks=76, xlt=None),
    dict(spt=64, bsh=5, exm=3, dsm=99, drm=63, al0=0x80, al1=0, off=1, cks=0,
         tracks=51, xlt=None),
]


class Disk:
    def __init__(self, g):
        self.g = g
        self.img = bytearray([0xE5]) * (g['tracks'] * g['spt'] * 128)
        self.rpb = 1 << g['bsh']                    # records per block
        self.ptrs = 16 if g['dsm'] < 256 else 8      # block pointers/entry
        self.next_block = bin(g['al0'] | g['al1'] << 8).count('1')
        self.entries = []

    def rec_off(self, rec):
        g = self.g
        track = g['off'] + rec // g['spt']
        sec = rec % g['spt']
        phys = g['xlt'][sec] - 1 if g['xlt'] else sec
        return (track * g['spt'] + phys) * 128

    def add(self, user, name, ext, data, attrs=()):
        g = self.g
        data = bytes(data)
        nrec = (len(data) + 127) // 128
        data = data.ljust(nrec * 128, b'\x1a')
        nblocks = (nrec + self.rpb - 1) // self.rpb
        blocks = list(range(self.next_block, self.next_block + nblocks))
        if blocks and blocks[-1] > g['dsm']:
            raise SystemExit('disk full')
        self.next_block += nblocks
        for i in range(nrec):
            block = blocks[i // self.rpb]
            rec = block * self.rpb + i % self.rpb
            o = self.rec_off(rec)
            self.img[o:o + 128] = data[i * 128:(i + 1) * 128]
        per_entry = self.ptrs * self.rpb          # records per directory entry
        lx = g['exm'] + 1                         # logical extents per entry
        nm = bytearray((name.upper().ljust(8) + ext.upper().ljust(3)).encode())
        if 'ro' in attrs:
            nm[8] |= 0x80
        if 'sys' in attrs:
            nm[9] |= 0x80
        nm = bytes(nm)
        k = 0
        while True:
            lo = k * per_entry
            n = max(0, min(per_entry, nrec - lo))
            j = max(0, (n - 1) // 128) if n else 0
            ex = k * lx + j
            rc = n - j * 128 if n else 0
            bl = blocks[lo // self.rpb:(lo + n + self.rpb - 1) // self.rpb]
            if self.ptrs == 16:
                m = bytes(bl) + bytes(16 - len(bl))
            else:
                m = b''.join(b.to_bytes(2, 'little') for b in bl)
                m += bytes(16 - len(m))
            ent = bytes([user]) + nm + bytes([ex & 0x1F, 0, ex >> 5, rc]) + m
            self.entries.append(ent)
            k += 1
            if lo + per_entry >= nrec:
                break

    def finish(self):
        g = self.g
        if len(self.entries) > g['drm'] + 1:
            raise SystemExit('directory full')
        d = b''.join(self.entries)
        d = d.ljust((g['drm'] + 1) * 32, b'\xe5')
        for r in range(len(d) // 128):
            o = self.rec_off(r)
            self.img[o:o + 128] = d[r * 128:(r + 1) * 128]
        return bytes(self.img)


def main(argv):
    disk = Disk(GEOMS[int(argv[1])])
    for spec in argv[3:]:
        left, host = spec.split('=', 1)
        user = 0
        if ':' in left:
            u, left = left.split(':', 1)
            user = int(u)
        left, *attrs = left.split(';')
        name, _, ext = left.partition('.')
        with open(host, 'rb') as f:
            disk.add(user, name, ext, f.read(), attrs)
    with open(argv[2], 'wb') as f:
        f.write(disk.finish())


if __name__ == '__main__':
    main(sys.argv)
