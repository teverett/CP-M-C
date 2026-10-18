#!/usr/bin/env python3
"""Build the QEMU system disk for the bare-metal i386 build.

usage: mkimage.py -o sys.img [-A a.img] [-B b.img] [-C c.img] [-s script]
                  [-t] [-e TRACK] [-x N:D:OFF:V]

Without -s the kernel runs interactively on the serial console.  See
src/os/i386/boot_hdr.h for the layout.
"""
import struct
import sys

sys.path.insert(0, __file__.rsplit('/', 1)[0])
import mkdisk  # noqa: E402

MAGIC = 0x434D5043
NO_SCRIPT = 0xFFFFFFFF
DRIVE_SECTORS = 2048
SCRIPT_LBA = 16 + 3 * DRIVE_SECTORS


def drive_lba(d):
    return 16 + d * DRIVE_SECTORS


def drive_size(d):
    g = mkdisk.GEOMS[d]
    return g['tracks'] * g['spt'] * 128


def build(drives, script=None, typeahead=False, err_track=-1, inject=None):
    """drives: {'A': bytes|None, ...} (None = present but blank, absent
    letters = not mounted).  inject: (count, drive, offset, value)."""
    present = 0
    for letter in drives:
        present |= 1 << (ord(letter) - ord('A'))
    ic, idrv, ioff, ival = inject if inject else (-1, 0, 0, 0)
    hdr = struct.pack('<IIiiIIIII', MAGIC, 1 if typeahead else 0, err_track,
                      ic, idrv, ioff, ival,
                      NO_SCRIPT if script is None else len(script), present)
    script_secs = 0 if script is None else (len(script) + 511) // 512
    img = bytearray((SCRIPT_LBA + script_secs) * 512)
    img[:len(hdr)] = hdr
    for letter, data in drives.items():
        d = ord(letter) - ord('A')
        if d >= 3:
            continue
        size = drive_size(d)
        data = bytes([0xE5]) * size if data is None else data.ljust(size, b'\xe5')
        o = drive_lba(d) * 512
        img[o:o + size] = data[:size]
    if script:
        o = SCRIPT_LBA * 512
        img[o:o + len(script)] = script
    return bytes(img)


def extract(img, letter):
    d = ord(letter) - ord('A')
    o = drive_lba(d) * 512
    return bytes(img[o:o + drive_size(d)])


def main(argv):
    out = script = None
    drives = {}
    typeahead = False
    err_track = -1
    inject = None
    i = 1
    while i < len(argv):
        a = argv[i]
        if a == '-o':
            out = argv[i + 1]; i += 2
        elif a in ('-A', '-B', '-C'):
            with open(argv[i + 1], 'rb') as f:
                drives[a[1]] = f.read()
            i += 2
        elif a == '-s':
            with open(argv[i + 1], 'rb') as f:
                script = f.read()
            i += 2
        elif a == '-t':
            typeahead = True; i += 1
        elif a == '-e':
            err_track = int(argv[i + 1]); i += 2
        elif a == '-x':
            n, d, o, v = (int(x) for x in argv[i + 1].split(':'))
            inject = (n, d, o, v); i += 2
        else:
            sys.exit(__doc__)
    if out is None:
        sys.exit(__doc__)
    with open(out, 'wb') as f:
        f.write(build(drives, script, typeahead, err_track, inject))


if __name__ == '__main__':
    main(sys.argv)
