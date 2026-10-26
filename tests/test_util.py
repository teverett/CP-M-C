#!/usr/bin/env python3
"""Smoke tests for the standalone CP/M utility COM programs."""
import os
import tempfile

import run_tests


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
UTIL_DIR = os.path.join(ROOT, 'build', 'util', 'z80')


def read_com(name):
    with open(os.path.join(UTIL_DIR, name + '.COM'), 'rb') as f:
        return f.read()


def hex_record(address, record_type, data):
    fields = [len(data), address >> 8, address & 0xff, record_type]
    fields.extend(data)
    checksum = (-sum(fields)) & 0xff
    return b':' + ''.join('%02X' % value for value in fields + [checksum]).encode()


def build_disks():
    long_assembly = (b'        ORG 0100H\r\n' +
                    b'        NOP\r\n' * 64 +
                    b'        END\r\n\x1a')
    files = [
        (0, name + '.COM', read_com(name))
        for name in ('ED', 'DUMP', 'LOAD', 'PIP', 'STAT', 'SUBMIT', 'MAC')
    ]
    files.extend([
        (0, 'SAMPLE.BIN', bytes(range(32))),
        (0, 'SOURCE.TXT', b'copy this text\r\nsecond line\r\n\x1a'),
        (0, 'ONE.TXT', b'submit parameter worked\r\n\x1a'),
        (0, 'TWO.TXT', b'second submitted line\r\n\x1a'),
        (0, 'RUN.SUB', b'TYPE $0.TXT\r\nTYPE TWO.TXT\r\n'),
        (0, 'HELLO.ASM',
         b'        ORG 0100H\r\n'
         b'START:  MVI A,41H\r\n'
         b'        OUT 0\r\n'
         b'        HLT\r\n'
         b'        END\r\n\x1a'),
        (0, 'LONG.ASM', long_assembly),
        (0, 'TEST.HEX',
         hex_record(0x0100, 0, b'\x00\x11\x22\x33') + b'\r\n' +
         hex_record(0, 1, b'') + b'\r\n\x1a'),
        (0, 'BAD.HEX',
         hex_record(0x0100, 0, b'\x00\x11')[:-2] + b'00\r\n' +
         hex_record(0, 1, b'') + b'\r\n\x1a'),
    ])
    return {
        'A': run_tests.make_disk(0, files),
        'B': run_tests.make_disk(1, [(0, 'SEED.TXT', b'B drive\r\n\x1a')]),
    }


def run(mode, command, disks):
    with tempfile.TemporaryDirectory(prefix='cpm-util-') as tmp:
        os.mkdir(os.path.join(tmp, mode))
        result = run_tests.run_one(mode, command, disks, (),
                                  os.path.join(tmp, mode))
    if result[2] != 0:
        raise AssertionError('%s returned %d: %r' % (mode, result[2], result[1]))
    return result[0]


def main():
    disks = build_disks()
    cases = [
        ('dump', b'DUMP SAMPLE.BIN\n', b'00 00 01 02 03 04 05 06', {'A': disks['A']}),
        ('pip', b'PIP COPY.TXT=SOURCE.TXT\nTYPE COPY.TXT\n',
         b'copy this text\r\nsecond line\r\n', {'A': disks['A']}),
        ('pip cross-drive',
         b'PIP B:COPY.TXT=A:SOURCE.TXT\nTYPE B:COPY.TXT\n',
         b'copy this text\r\nsecond line\r\n', disks),
        ('pip wildcard',
         b'PIP B:=A:*.TXT\nTYPE B:ONE.TXT\nTYPE B:SOURCE.TXT\n',
         (b'submit parameter worked\r\n',
          b'copy this text\r\nsecond line\r\n'), disks),
        ('pip wildcard destination',
         b'PIP B:*.BAK=A:*.TXT\nTYPE B:ONE.BAK\n',
         b'submit parameter worked\r\n', disks),
        ('load', b'LOAD TEST.HEX\nDUMP TEST.COM\n',
         b'00 11 22 33', {'A': disks['A']}),
        ('load bad checksum', b'LOAD BAD.HEX\n', b'INVALID HEX FILE',
         {'A': disks['A']}),
        ('stat', b'STAT\nSTAT DSK:\n', b'ED.COM', {'A': disks['A']}),
        ('stat disk', b'STAT DSK:\n', b'FREE BLOCKS', {'A': disks['A']}),
        ('submit', b'SUBMIT RUN one\n',
         (b'submit parameter worked\r\n', b'second submitted line\r\n'),
         {'A': disks['A']}),
        ('mac', b'MAC HELLO\nTYPE HELLO.HEX\n',
         b':050100003E41D3007632', {'A': disks['A']}),
        ('mac multi-record', b'MAC LONG\nTYPE LONG.HEX\n',
         tuple(hex_record(0x0100 + offset, 0, bytes(16)) + b'\r\n'
               for offset in range(0, 64, 16)) +
         (hex_record(0, 1, b'') + b'\r\n',),
         {'A': disks['A']}),
    ]
    for mode in ('port', 'z80', 'i386'):
        for name, command, expected, case_disks in cases:
            output = run(mode, command, case_disks)
            expectations = expected if isinstance(expected, tuple) else (expected,)
            missing = [item for item in expectations if item not in output]
            if missing:
                raise AssertionError('%s/%s missing expected output %r: %r'
                                     % (mode, name, missing, output[-300:]))
            print('ok %s/%s' % (mode, name))


if __name__ == '__main__':
    main()
