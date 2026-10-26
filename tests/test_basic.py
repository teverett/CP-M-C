#!/usr/bin/env python3
"""Execute BASIC.COM on the hosted, native Z80, and i386 targets."""
import os
from pathlib import Path
import re
import tempfile

import run_tests


ROOT = Path(__file__).resolve().parent.parent


PROGRAMS = {
    'ARITH.BAS': '''
LET TOTAL = 0
FOR INDEX = 1 TO 5
TOTAL = TOTAL + INDEX
NEXT INDEX
IF TOTAL = 15 THEN PRINT "ARITH OK" ELSE PRINT "FAIL"
IF 2 + 3 * 4 = 14 THEN PRINT "PRECEDENCE OK"
IF 2 ^ 3 = 8 THEN PRINT "POWER OK"
END
''',
    'FLOW.BAS': '''
10 GOSUB 100
20 IF A = 3 THEN GOTO 40 ELSE PRINT "FAIL"
30 STOP
40 ON 2 GOTO 50,60
50 PRINT "FAIL": STOP
60 PRINT "FLOW OK": END
100 A = 3
110 RETURN
''',
    'STRINGS.BAS': '''
NAME$ = "Hello"
IF LEN(NAME$) = 5 THEN PRINT "LENGTH OK"
IF LEFT$(NAME$,2) + RIGHT$(NAME$,2) = "Helo" THEN PRINT "STRINGS OK"
IF MID$(NAME$,2,3) = "ell" THEN PRINT "MID OK"
IF CHR$(65) = "A" THEN PRINT "CHR OK"
END
''',
    'DATA.BAS': '''
DIM VALUES(3)
DATA 2,4,6
FOR I = 1 TO 3
READ VALUES(I)
NEXT I
IF VALUES(1)+VALUES(2)+VALUES(3) = 12 THEN PRINT "ARRAY DATA OK"
RESTORE
READ X
IF X = 2 THEN PRINT "RESTORE OK"
END
''',
    'FLOAT.BAS': '''
X = 1.25 + 2.5
IF ABS(X - 3.75) < .001 THEN PRINT "FLOAT OK"
IF ABS(SQR(9)-3) < .001 THEN PRINT "SQRT OK"
IF INT(3.9) = 3 THEN PRINT "INT OK"
IF ABS(SIN(0))+ABS(COS(0)-1) < .001 THEN PRINT "TRIG OK"
END
''',
    'IF.BAS': '''
A = 0
IF 0 THEN A = 1: PRINT "FAIL"
IF A = 0 THEN PRINT "FALSE BRANCH OK"
IF 1 THEN A = 2: A = A + 1 ELSE A = 9
IF A = 3 THEN PRINT "TRUE BRANCH OK"
END
''',
    'INPUT.BAS': '''
INPUT "VALUE"; X
IF X = 7 THEN PRINT "INPUT OK" ELSE PRINT "FAIL"
END
''',
    'LOOPS.BAS': '''
TOTAL = 0
FOR I = 3 TO 1 STEP -1
FOR J = 1 TO 2
TOTAL = TOTAL + I
NEXT J
NEXT I
IF TOTAL = 12 THEN PRINT "NESTED LOOPS OK"
FOR K = 2 TO 1
TOTAL = TOTAL + 100
NEXT K
IF TOTAL = 112 THEN PRINT "SINGLE ITERATION OK"
END
''',
    'CONT.BAS': '''
REMARK source order and continued statement
ANSWER = 1 + \\
2 + 3
IF ANSWER = 6 THEN PRINT "CONTINUATION OK"
PRINT "COLON:IN STRING"
END
''',
    'ZERO.BAS': 'PRINT 1 / 0\nEND\n',
    'BAD.BAS': 'LET X = (1 + )\nPRINT "FAIL"\nEND\n',
    'BOUNDS.BAS': 'DIM A(2)\nPRINT A(3)\nPRINT "FAIL"\nEND\n',
    'UNSUP.BAS': 'FILE "TEST.TXT"\nPRINT "FAIL"\nEND\n',
    'DEPTH.BAS': 'PRINT ' + '(' * 20 + '1' + ')' * 20 + '\nEND\n',
    'LARGE.BAS': 'REMARK ' + 'x' * 6200 + '\nEND\n',
}


def main():
    link_map = (ROOT / 'build/util/z80/basic.map').read_text()
    symbols = {name: int(value, 16) for value, name in
               re.findall(r'^\s+([0-9A-Fa-f]{8})\s+([sl]__\w+)\s',
                          link_map, re.MULTILINE)}
    code_end = max(symbols['s__CODE'] + symbols['l__CODE'],
                   symbols['s__HOME'] + symbols['l__HOME'])
    data_end = symbols['s__DATA'] + symbols['l__DATA']
    if code_end > symbols['s__DATA'] or data_end > 0x9B00:
        raise AssertionError('BASIC code/data overlap or insufficient native Z80 stack')
    files = [(0, name + '.COM', (ROOT / ('build/util/z80/' + name + '.COM')).read_bytes())
             for name in ('BASIC', 'BASICBAS')]
    files.extend((0, name, source.strip().encode('ascii') + b'\r\n\x1a')
                 for name, source in PROGRAMS.items())
    disk = run_tests.make_disk(0, files)
    other_disk = run_tests.make_disk(
        1, [(0, 'REMOTE.BAS', b'PRINT "REMOTE SOURCE OK"\r\nEND\r\n\x1a')])
    cases = [
        ('arithmetic', b'BASIC ARITH\n',
         (b'ARITH OK', b'PRECEDENCE OK', b'POWER OK')),
        ('control flow', b'BASIC FLOW.BAS\n', (b'FLOW OK',)),
        ('strings', b'BASIC STRINGS\n',
         (b'LENGTH OK', b'STRINGS OK', b'MID OK', b'CHR OK')),
        ('arrays and data', b'BASIC DATA\n',
         (b'ARRAY DATA OK', b'RESTORE OK')),
        ('floating point', b'BASIC FLOAT\n',
         (b'FLOAT OK', b'SQRT OK', b'INT OK', b'TRIG OK')),
        ('conditional scope', b'BASIC IF\n',
         (b'FALSE BRANCH OK', b'TRUE BRANCH OK')),
        ('console input', b'BASIC INPUT\n7\n', (b'INPUT OK',)),
        ('nested and single-iteration loops', b'BASIC LOOPS\n',
         (b'NESTED LOOPS OK', b'SINGLE ITERATION OK')),
        ('source scanning', b'BASIC CONT\n',
         (b'CONTINUATION OK', b'COLON:IN STRING')),
        ('drive-qualified source', b'BASIC B:REMOTE\n',
         (b'REMOTE SOURCE OK',)),
        ('runtime fallback to A', b'B:\nA:BASIC REMOTE\nA:\n',
         (b'REMOTE SOURCE OK',)),
    ]
    errors = [
        ('missing filename', b'BASIC\n'),
        ('unsupported extension', b'BASIC ARITH.TXT\n'),
        ('unexpected arguments', b'BASIC ARITH EXTRA\n'),
        ('division by zero', b'BASIC ZERO\n'),
        ('syntax error', b'BASIC BAD\n'),
        ('array bounds', b'BASIC BOUNDS\n'),
        ('missing source', b'BASIC MISSING\n'),
        ('unsupported statement', b'BASIC UNSUP\n'),
        ('expression depth', b'BASIC DEPTH\n'),
        ('source size', b'BASIC LARGE\n'),
    ]
    for mode in ('port', 'z80', 'i386'):
        for name, overlays, diagnostic in (
                ('missing', [], b'ERROR: Missing BASIC runtime'),
                ('empty', [(0, 'BASICBAS.COM', b'')],
                 b'ERROR: Cannot load BASIC runtime'),
                ('oversized', [(0, 'BASICBAS.COM', bytes(0x9B00))],
                 b'ERROR: Cannot load BASIC runtime')):
            with tempfile.TemporaryDirectory(prefix='cpm-basic-overlay-') as tmp:
                work = os.path.join(tmp, mode)
                os.mkdir(work)
                output, _, code, *_ = run_tests.run_one(
                    mode, b'BASIC DEMO\nBASIC\n',
                    {'A': run_tests.make_disk(0, [files[0]] + overlays)}, (), work)
            if code or diagnostic not in output or b'ERROR: Usage:' not in output:
                raise AssertionError('%s/%s runtime: %r'
                                     % (mode, name, output[-1500:]))
            print('ok %s/basic %s overlay diagnostic' % (mode, name))
        with tempfile.TemporaryDirectory(prefix='cpm-basic-image-') as tmp:
            work = os.path.join(tmp, mode)
            os.mkdir(work)
            output, _, code, *_ = run_tests.run_one(
                mode, b'BASIC DEMO\n',
                {'A': (ROOT / 'build/disks/utilities-A.img').read_bytes()},
                (), work)
        if code or b'SUBROUTINE OK' not in output or b'ERROR' in output:
            raise AssertionError('%s/installed BASIC DEMO: %r'
                                 % (mode, output[-1500:]))
        print('ok %s/basic installed filesystem demo' % mode)
        for name, command, expected in cases:
            with tempfile.TemporaryDirectory(prefix='cpm-basic-') as tmp:
                work = os.path.join(tmp, mode)
                os.mkdir(work)
                output, _, code, *_ = run_tests.run_one(
                    mode, command, {'A': disk, 'B': other_disk}, (), work)
            if code or any(text not in output for text in expected) or b'FAIL' in output:
                raise AssertionError('%s/%s: exit=%r output=%r'
                                     % (mode, name, code, output[-1500:]))
            print('ok %s/basic %s' % (mode, name))
        for name, command in errors:
            with tempfile.TemporaryDirectory(prefix='cpm-basic-') as tmp:
                work = os.path.join(tmp, mode)
                os.mkdir(work)
                output, _, code, *_ = run_tests.run_one(
                    mode, command + b'BASIC FLOAT\n', {'A': disk}, (), work)
            if (code or b'ERROR' not in output.upper() or b'FAIL' in output
                    or b'FLOAT OK' not in output):
                raise AssertionError('%s/%s: exit=%r output=%r'
                                     % (mode, name, code, output[-1500:]))
            print('ok %s/basic %s and recovery' % (mode, name))


if __name__ == '__main__':
    main()
