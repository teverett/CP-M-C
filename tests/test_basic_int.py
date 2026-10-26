#!/usr/bin/env python3
"""Run files emitted by the preserved BASIC-E compiler on the C INT runtime."""
from pathlib import Path
import re
import tempfile

import run_tests


ROOT = Path(__file__).resolve().parent.parent
PROGRAMS = {
    'INTMATH': ('''
TOTAL=0
FOR I=1 TO 5
TOTAL=TOTAL+I
NEXT I
IF TOTAL=15 THEN PRINT "INT LOOP OK"
IF ABS(SQR(9)-3)<.001 THEN PRINT "INT MATH OK"
X=1.25+.5
IF ABS(X-1.75)<.001 THEN PRINT "INT FLOAT OK"
IF ABS(2^3-8)<.001 THEN PRINT "INT POWER OK"
PRINT 1.25
IF NOT 0=-1 THEN PRINT "INT NOT OK"
IF (65536 OR 1)=65537 THEN PRINT "INT LOGIC OK"
END
''', (b'INT LOOP OK', b'INT MATH OK', b'INT FLOAT OK',
      b'INT POWER OK', b'1.25', b'INT NOT OK', b'INT LOGIC OK')),
    'INTSTR': ('''
A$="HELLO"
B$=LEFT$(A$,2)+RIGHT$(A$,2)
IF B$="HELO" THEN PRINT "INT STRING OK"
IF MID$(A$,2,3)="ELL" THEN PRINT "INT MID OK"
IF STR$(-12)="-12" THEN PRINT "INT STR OK"
END
''', (b'INT STRING OK', b'INT MID OK', b'INT STR OK')),
    'INTDATA': ('''
DIM A(3)
DATA 2,4,6
FOR I=1 TO 3
READ A(I)
NEXT I
IF A(1)+A(2)+A(3)=12 THEN PRINT "INT ARRAY OK"
RESTORE
READ X
IF X=2 THEN PRINT "INT RESTORE OK"
END
''', (b'INT ARRAY OK', b'INT RESTORE OK')),
    'INTCALL': ('''
10 GOSUB 100
20 ON 2 GOTO 30,40
30 GOTO 200
40 IF A=7 THEN PRINT "INT CALL OK"
50 GOTO 200
100 A=7
110 RETURN
200 END
''', (b'INT CALL OK',)),
    'INTDEF': ('''
DEF FNA(X,Y)=X*2+Y
IF FNA(3,4)=10 THEN PRINT "INT DEF OK"
END
''', (b'INT DEF OK',)),
    'INTFILE': ('''
F$="RESULT.TXT"
FILE F$
PRINT #1; "INT FILE OK",12
CLOSE 1
END
''', (b'INT FILE OK',)),
}


def compiled_disk():
    files = [(0, name + '.COM',
                  (ROOT / ('build/util/z80/' + name + '.COM')).read_bytes())
                 for name in ('BASIC', 'BASICBAS', 'BASICINT')]
    geometry = run_tests.mkdisk.Disk(run_tests.mkdisk.GEOMS[0])
    compiler = (ROOT / 'orig/basic-e/BASIC.COM').read_bytes()
    # The preserved compiler does not reliably restart in one boot.
    for name, (text, _) in PROGRAMS.items():
        source = text.strip().replace('\n', '\r\n').encode() + b'\r\n\x1a'
        with tempfile.TemporaryDirectory(prefix='cpm-int-compile-') as tmp:
            out, err, code, images, _ = run_tests.run_one(
                'port', ('BECOMP %s\n' % name).encode(),
                {'A': run_tests.make_disk(0, [(0, 'BECOMP.COM', compiler),
                                             (0, name + '.BAS', source)])}, (), tmp)
        if code or b'0 ERRORS DETECTED' not in out:
            raise AssertionError('Original compiler failed: %r %r' % (out, err))
        image = images['A']
        entries = []
        for index in range(64):
            offset = geometry.rec_off(index // 4) + (index % 4) * 32
            entry = image[offset:offset + 32]
            if entry[0] == 0 and entry[1:12] == (name.ljust(8) + 'INT').encode():
                entries.append(entry)
        if not entries:
            raise AssertionError('Original compiler produced no %s.INT' % name)
        data = bytearray()
        for entry in sorted(entries, key=lambda e: e[12]):
            for record in range(entry[15]):
                block = entry[16 + record // 8]
                offset = geometry.rec_off(block * 8 + record % 8)
                data.extend(image[offset:offset + 128])
        files.append((0, name + '.INT', bytes(data)))
    files.extend([(0, 'BAD.INT', b'*' + bytes([0, 1, 0, 0, 0, 0, 126, 127])),
                  (0, 'SHORT.INT', b'123$')])
    return run_tests.make_disk(0, files)


def main():
    link_map = (ROOT / 'build/util/z80/basic-int.map').read_text()
    symbols = {name: int(value, 16) for value, name in
               re.findall(r'^\s+([0-9A-Fa-f]{8})\s+([sl]__\w+)\s',
                          link_map, re.MULTILINE)}
    code_end = max(symbols['s__CODE'] + symbols['l__CODE'],
                   symbols['s__HOME'] + symbols['l__HOME'])
    data_end = symbols['s__DATA'] + symbols['l__DATA']
    if code_end > symbols['s__DATA'] or data_end > 0x9B00:
        raise AssertionError('INT code/data overlap or insufficient native Z80 stack')
    disk = compiled_disk()
    for mode in ('port', 'z80', 'i386'):
        for name, (_, expected) in PROGRAMS.items():
            script = ('BASIC %s.INT\n' % name).encode()
            if name == 'INTFILE':
                script += b'TYPE RESULT.TXT\n'
            with tempfile.TemporaryDirectory(prefix='cpm-int-run-') as tmp:
                out, err, code, _, _ = run_tests.run_one(
                    mode, script, {'A': disk}, (), tmp)
            if code or b'ERROR' in out or any(x not in out for x in expected):
                raise AssertionError('%s/%s: %r %r' % (mode, name, out, err))
            print('ok %s/original %s.INT' % (mode, name))
        for name in ('BAD', 'SHORT', 'MISSING'):
            with tempfile.TemporaryDirectory(prefix='cpm-int-error-') as tmp:
                out, err, code, _, _ = run_tests.run_one(
                    mode, ('BASIC %s.INT\nBASIC INTMATH.INT\n' % name).encode(),
                    {'A': disk}, (), tmp)
            if code or b'ERROR' not in out or b'INT LOOP OK' not in out:
                raise AssertionError('%s/%s: %r %r' % (mode, name, out, err))
            print('ok %s/%s.INT error and recovery' % (mode, name))


if __name__ == '__main__':
    main()
