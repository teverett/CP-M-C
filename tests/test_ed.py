#!/usr/bin/env python3
"""Focused smoke tests for the standalone BDOS-backed ED COM program."""
import os
import tempfile

import run_tests


DRIVE_A = run_tests.make_disk(0, [
    (0, 'ED.COM', open(os.path.join(os.path.dirname(__file__),
                                    '..', 'build', 'util', 'z80', 'ED.COM'),
                       'rb').read()),
    (0, 'SAMPLE.TXT', b'first line\r\nsecond line\r\n\x1a'),
])


def run_ed(mode, script):
    with tempfile.TemporaryDirectory(prefix='cpm-ed-') as tmp:
        os.mkdir(os.path.join(tmp, mode))
        result = run_tests.run_one(mode, script, {'A': DRIVE_A}, (),
                                  os.path.join(tmp, mode))
    if result[2] != 0:
        raise AssertionError('%s returned %d: %r' % (mode, result[2], result[1]))
    return result[0]


def main():
    cases = [
        ('replace and save',
         b'ed sample.txt\nsfirst\x1achanged\x1a\nE\ntype sample.txt\n',
         b'changed line\r\nsecond line\r\n'),
        ('quit without saving',
         b'ed sample.txt\nQ\ny\ntype sample.txt\n',
         b'first line\r\nsecond line\r\n'),
        ('transfer and read',
         b'ed sample.txt\nX\nB\nr\nE\ntype sample.txt\n',
         b'first line\r\nfirst line\r\nsecond line\r\n'),
        ('uppercase insert',
         b'ed sample.txt\nIlower text\x0c\x1a\nE\ntype sample.txt\n',
         b'LOWER TEXT\r\nfirst line\r\n'),
        ('lowercase insert',
         b'ed sample.txt\niMiXeD text\x0c\x1a\nE\ntype sample.txt\n',
         b'MiXeD text\r\nfirst line\r\n'),
    ]
    for mode in ('port', 'z80', 'i386'):
        for label, script, expected in cases:
            output = run_ed(mode, script)
            if expected not in output:
                raise AssertionError('%s/%s missing expected text: %r'
                                     % (mode, label, output[-200:]))
            if b'DISK OR DIRECTORY FULL' in output:
                raise AssertionError('%s/%s reported a disk error'
                                     % (mode, label))
            print('ok %s/%s' % (mode, label))


if __name__ == '__main__':
    main()
