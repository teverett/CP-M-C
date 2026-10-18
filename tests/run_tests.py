#!/usr/bin/env python3
"""Differential tests: the original 8080 CCP+BDOS (assembled from orig/os/ and
run on the emulated CPU) against the C port.  The same console script and
the same starting disk images go into both; console output, exit status and
the final disk images must be identical.

usage: run_tests.py [-v] [-k substring] [--fuzz-seeds N]
"""
import os
import random
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD = os.path.join(ROOT, 'build')
CPM = os.path.join(BUILD, 'cpm')
REF = os.path.join(BUILD, 'ref')
Z80BIN = os.path.join(BUILD, 'z80', 'cpm.bin')
I386ELF = os.path.join(BUILD, 'i386', 'kernel.elf')
QEMU = os.environ.get('QEMU386', 'qemu-system-i386')
TARGETS = ['port', 'z80', 'i386']  # compared against the reference
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import mkdisk  # noqa: E402
import mkimage  # noqa: E402


# ---------------------------------------------------------------------------
# helpers

def asm(name):
    src = os.path.join(ROOT, 'tests', 'asm', name + '.asm')
    out = os.path.join(BUILD, 'tests', name + '.com')
    sym = os.path.join(BUILD, 'tests', name + '.sym')
    os.makedirs(os.path.dirname(out), exist_ok=True)
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'asm80.py'),
                    '-o', out, '-s', sym, src], check=True,
                   stdout=subprocess.DEVNULL)
    syms = {}
    with open(sym) as f:
        for line in f:
            k, v = line.split()
            syms[k] = int(v, 16)
    with open(out, 'rb') as f:
        return f.read(), syms


def make_disk(kind, files):
    """files: list of (user, 'NAME.EXT', bytes)"""
    d = mkdisk.Disk(mkdisk.GEOMS[kind])
    for user, fn, data in files:
        fn, *attrs = fn.split(';')
        name, _, ext = fn.partition('.')
        d.add(user, name, ext, data, attrs)
    return d.finish()


def text(n, tag):
    return b''.join(b'%s line %04d\r\n' % (tag, i) for i in range(n)) + b'\x1a'


def pattern(n, seed):
    r = random.Random(seed)
    return bytes(r.randrange(256) for _ in range(n))


def run_i386(script, disks, flags, cwd):
    typeahead = '-t' in flags
    err_track = -1
    inject = None
    fl = list(flags)
    i = 0
    while i < len(fl):
        if fl[i] == '-e':
            err_track = int(fl[i + 1]); i += 2
        elif fl[i] == '-x':
            n, d, o, v = (int(x) for x in fl[i + 1].split(':'))
            inject = (n, d, o, v); i += 2
        else:
            i += 1                    # -t, -m are handled/ignored
    img = mkimage.build(disks, script, typeahead, err_track, inject)
    sysimg = os.path.join(cwd, 'sys.img')
    with open(sysimg, 'wb') as f:
        f.write(img)
    lst = os.path.join(cwd, 'list')
    diag = os.path.join(cwd, 'diag')
    args = [QEMU, '-display', 'none', '-monitor', 'none', '-no-reboot',
            '-m', '16', '-kernel', I386ELF,
            '-drive', 'file=%s,format=raw,if=ide' % sysimg,
            '-chardev', 'stdio,id=c0', '-serial', 'chardev:c0',
            '-serial', 'file:' + lst, '-serial', 'file:' + diag,
            '-device', 'isa-debug-exit,iobase=0xf4,iosize=0x04']
    try:
        r = subprocess.run(args, capture_output=True, timeout=300,
                           stdin=subprocess.DEVNULL)
        out, rc = r.stdout, r.returncode
        # isa-debug-exit: status = ((0x20 + code) << 1) | 1; anything lower
        # is QEMU itself failing (e.g. it could not load the kernel)
        if rc & 1 and rc >= 0x41:
            rc = (rc >> 1) - 0x20
        else:
            err += b'\n[qemu exit status %d]' % rc
            rc = -1000 - rc
        err = r.stderr
    except subprocess.TimeoutExpired:
        out, err, rc = b'', b'TIMEOUT', -1
    with open(diag, 'rb') as f:
        err += f.read()
    with open(sysimg, 'rb') as f:
        final = f.read()
    images = {l: mkimage.extract(final, l) for l in disks}
    with open(lst, 'rb') as f:
        listing = f.read()
    return out, err, rc, images, listing


def run_one(mode, script, disks, flags, cwd):
    if mode == 'i386':
        return run_i386(script, disks, flags, cwd)
    args = [CPM]
    if mode == 'ref':
        args += ['-r', REF]
    elif mode == 'z80':
        args += ['-z', Z80BIN]
    args += list(flags)
    inp = os.path.join(cwd, 'in')
    with open(inp, 'wb') as f:
        f.write(script)
    args += ['-i', inp, '-l', os.path.join(cwd, 'list')]
    for letter, data in disks.items():
        p = os.path.join(cwd, letter + '.img')
        if data is not None:
            with open(p, 'wb') as f:
                f.write(data)
        args += ['-' + letter, p]
    try:
        r = subprocess.run(args, capture_output=True, timeout=300)
        out, err, rc = r.stdout, r.stderr, r.returncode
    except subprocess.TimeoutExpired:
        out, err, rc = b'', b'TIMEOUT', -1
    images = {}
    for letter in disks:
        p = os.path.join(cwd, letter + '.img')
        if os.path.exists(p):
            with open(p, 'rb') as f:
                images[letter] = f.read()
    lst = b''
    if os.path.exists(os.path.join(cwd, 'list')):
        with open(os.path.join(cwd, 'list'), 'rb') as f:
            lst = f.read()
    return out, err, rc, images, lst


def first_diff(a, b):
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return i
    return min(len(a), len(b))


def show(b, pos, w=60):
    lo = max(0, pos - w)
    return repr(b[lo:pos + w])


class Case:
    def __init__(self, name, script, disks=None, flags=(), rc=0, only=None):
        self.rc = rc
        self.only = only          # restrict to these targets (layout-specific)
        self.name = name
        self.script = script
        self.disks = disks or {}
        self.flags = flags


def compare(case, ro, rp, label):
    problems = []
    if ro[0] != rp[0]:
        i = first_diff(ro[0], rp[0])
        problems.append('%s: console differs at %d:\n   ref : %s\n   %s: %s'
                        % (label, i, show(ro[0], i), label, show(rp[0], i)))
        if rp[1]:
            problems.append('%s: stderr/diagnostics: %r' % (label, rp[1][-300:]))
    if ro[2] != rp[2]:
        problems.append('%s: exit status ref=%d got=%d (%r / %r)'
                        % (label, ro[2], rp[2], ro[1][-200:], rp[1][-200:]))
    for letter in ro[3]:
        a, b = ro[3][letter], rp[3].get(letter)
        if a != b:
            i = first_diff(a, b or b'')
            problems.append('%s: disk %s differs at byte %d (sector %d)'
                            % (label, letter, i, i // 128))
    if ro[4] != rp[4]:
        problems.append('%s: list output differs' % label)
    return problems


def check(case, verbose):
    tmp = tempfile.mkdtemp(prefix='cpmtest')
    try:
        res = {}
        for mode in ['ref'] + [t for t in TARGETS
                               if case.only is None or t in case.only]:
            cwd = os.path.join(tmp, mode)
            os.mkdir(cwd)
            res[mode] = run_one(mode, case.script, case.disks, case.flags, cwd)
        ro = res['ref']
        problems = []
        for mode in TARGETS:
            if case.only is None or mode in case.only:
                problems += compare(case, ro, res[mode], mode)
        if ro[2] != case.rc and not problems:
            problems.append('both exited with status %d (expected %d): %r'
                            % (ro[2], case.rc, ro[1]))
        if verbose or problems:
            print('  [%s] %s' % ('FAIL' if problems else 'ok', case.name))
        for p in problems:
            print('     ' + p)
        if verbose and not problems:
            print('     %d bytes of console output' % len(ro[0]))
        return not problems
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


# ---------------------------------------------------------------------------
# directed CCP / console cases

def ccp_cases():
    hello, _ = asm('hello')
    retprog, _ = asm('retprog')
    jmpccp, _ = asm('jmpccp')
    jmpccp2, _ = asm('jmpccp2')
    t1 = text(30, b'alpha')
    t2 = text(300, b'beta')
    big = pattern(40000, 1)
    a0 = [
        (0, 'RETPROG.COM', retprog), (0, 'JMPCCP.COM', jmpccp),
        (0, 'JMPCCP2.COM', jmpccp2), (0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1), (0, 'BETA.TXT', t2),
        (0, 'BIG.DAT', big), (1, 'U1.TXT', t1), (0, 'SECRET.TXT', t1),
        (0, 'SYSF.TXT', t1), (0, 'EMPTY.DAT', b''),
        (0, 'A.B', b'x'), (0, 'LONGNAME.EXT', b'y' * 3000),
    ]
    a = make_disk(0, a0)
    b = make_disk(1, [(0, 'HELLO.COM', hello), (0, 'BTEXT.TXT', t1),
                      (0, 'BBIG.DAT', pattern(70000, 2))])
    c = make_disk(2, [(0, 'CTEXT.TXT', t2), (0, 'CBIG.DAT', pattern(150000, 3))])
    D = {'A': a, 'B': b, 'C': c}

    def S(*lines):
        return b''.join(l if isinstance(l, bytes) else l.encode()
                        for l in lines)

    cases = [
        Case('ccp/prompt-eof', b'', D),
        Case('ccp/dir', S('dir\n', 'dir *.com\n', 'dir a*.*\n', 'dir ?l*.t??\n',
                          'dir b:\n', 'dir c:*.dat\n', 'dir nosuch\n',
                          'dir b:*.*\n', 'dir x*\n'), D),
        Case('ccp/dir-user', S('dir\n', 'user 1\n', 'dir\n', 'user 0\n',
                               'user 2\n', 'dir\n', 'user 16\n', 'user x\n',
                               'user\n', 'user 255\n', 'user 1 2\n'), D),
        Case('ccp/type', S('type alpha.txt\n', 'type beta.txt\n',
                           'type nosuch.txt\n', 'type *.txt\n',
                           'type b:btext.txt\n', 'type c:ctext.txt\n',
                           'type big.dat\n'), D),
        Case('ccp/type-break', S('type beta.txt\n', 'x', 'dir\n'), D, ['-t']),
        Case('ccp/dir-break', S('dir\n', 'x', 'dir\n'), D, ['-t']),
        Case('ccp/era', S('era alpha.txt\n', 'dir\n', 'era nosuch\n',
                          'era a*.*\n', 'era b:bt*.*\n', 'dir b:\n'), D),
        Case('ccp/era-all-y', S('era *.*\n', 'y\n', 'dir\n', 'dir b:\n'), D),
        Case('ccp/era-all-n', S('era *.*\n', 'n\n', 'dir\n'), D),
        Case('ccp/era-all-long', S('era *.*\n', 'yes\n', 'dir\n'), D),
        Case('ccp/era-bdrive', S('era b:*.*\n', 'Y\n', 'dir b:\n', 'dir\n'), D),
        Case('ccp/ren', S('ren new.txt=alpha.txt\n', 'dir\n',
                          'ren beta.txt=nosuch\n', 'ren hello.com=big.dat\n',
                          'ren x.y=\n', 'ren x.y\n', 'ren a*.x=b.c\n',
                          'ren b:z.txt=b:btext.txt\n', 'dir b:\n',
                          'ren b:q.q=a:big.dat\n', 'ren a:q.q=b:big.dat\n',
                          'ren neu.dat<big.dat\n', 'dir\n'), D),
        Case('ccp/save', S('save 2 two.dat\n', 'save 0 zero.dat\n',
                           'dir\n', 'type two.dat\n', 'save 300 huge.dat\n',
                           'save x y.z\n', 'save 1 b:one.dat\n', 'dir b:\n',
                           'save 1 hello.com\n', 'dir\n'), D),
        Case('ccp/drive-switch', S('b:\n', 'dir\n', 'c:\n', 'dir\n', 'a:\n',
                                   'dir\n', 'b:hello\n', 'c:\n', 'hello\n',
                                   'a:\n'), D),
        Case('ccp/drive-bad', S('d:\n', 'dir d:\n', 'type d:x\n', 'e:\n',
                                'dir\n'), D),
        Case('ccp/run-com', S('hello\n', 'hello a b c\n', 'hello x:y.z q.r\n',
                              'hello b:file.txt *.* ??\n', 'b:hello\n',
                              'hello  =  <>\n', 'a:hello foo\n'), D),
        Case('ccp/run-com-user', S('user 3\n', 'hello\n', 'user 0\n',
                                   'hello\n'), D),
        Case('ccp/bad-command', S('nosuch\n', 'foo bar\n', 'a:nosuch\n',
                                  'foo.com\n', 'hello.txt\n', '*\n', '?x\n',
                                  'dir*\n', 'a:b:c\n', 'x;y\n', 'x=y\n',
                                  'x.y.z\n', 'abcdefghijkl\n',
                                  'abcdefgh.ijkl\n', 'dir  ,x\n'), D),
        Case('ccp/ctl-chars-in-name', S('dir\x01\n', 'abc\x07d\n'), D),
        Case('ccp/empty-lines', S('\n', '   \n', '\n\n', ' dir\n',
                                  'dir \n', '\tdir\n'), D),
        Case('ccp/lowercase', S('DIR\n', 'Dir\n', 'tYpE ALPHA.txt\n'), D),
        Case('ccp/big-load', S('big\n', 'big.dat\n'), D),
        Case('ccp/com-too-big',
             S('hello\n'), {'A': make_disk(0, [(0, 'HELLO.COM', b'\0' * 56000)]),
                            'B': b, 'C': c}),
        Case('ccp/empty-file', S('empty\n', 'type empty.dat\n', 'dir\n'), D),
        Case('ccp/exit-ret', S('retprog\n', 'retprog\n', 'dir\n'), D),
        Case('ccp/exit-jmp-ccp', S('jmpccp\n', 'dir\n', 'jmpccp2\n'), D),
        Case('ccp/exit-jmp-ccp-user', S('user 2\n', 'jmpccp\n', 'dir\n',
                                        'b:\n', 'jmpccp\n', 'dir\n'), D),
        Case('ccp/disk-full', S('save 150 f1.dat\n', 'save 150 f2.dat\n',
                                'save 150 f3.dat\n', 'save 150 f4.dat\n',
                                'save 150 f5.dat\n', 'save 150 f6.dat\n',
                                'save 150 f7.dat\n', 'save 150 f8.dat\n',
                                'dir\n',
                                'era f*.dat\n', 'dir\n'), D),
        Case('ccp/dir-full', S(*['save 0 f%02d.dat\n' % i for i in range(70)],
                               'dir\n'), D),
        Case('ccp/many-extents',
             S('type longname.ext\n', 'dir b:\n', 'type b:bbig.dat\n',
               'type c:cbig.dat\n'), D, ['-m', '400000000']),
    ]

    # --- line editing ---
    ed = [
        ('edit/backspace', b'dix\x08r\n'),
        ('edit/backspace-start', b'\x08\x08dir\n'),
        ('edit/rubout', b'di\x7fir\x7f\x7fdir\n'),
        ('edit/rubout-empty', b'\x7f\x7fdir\n'),
        ('edit/ctl-u', b'abc\x15dir\n'),
        ('edit/ctl-x', b'abc\x18dir\n'),
        ('edit/ctl-x-empty', b'\x18dir\n'),
        ('edit/ctl-r', b'dir\x12\n'),
        ('edit/ctl-r-2', b'di\x12r\x12\n'),
        ('edit/ctl-e', b'di\x05r\n'),
        ('edit/ctl-p', b'\x10dir\n\x10dir\n'),
        ('edit/ctl-c-start', b'\x03dir\n'),
        ('edit/ctl-c-mid', b'di\x03r\n'),
        ('edit/tab', b'di\tr\n\tdir\n'),
        ('edit/tab-backspace', b'ab\tc\x08\x08\x08dir\n'),
        ('edit/ctl-char-echo', b'a\x01b\x1fc\n'),
        ('edit/ctl-char-backspace', b'a\x01\x01\x08\x08\x08dir\n'),
        ('edit/ctl-char-rubout', b'a\x01\x7f\x7fdir\n'),
        ('edit/ctl-r-ctl', b'a\x01b\x12\n'),
        ('edit/long-line', b'dir ' + b'a' * 200 + b'\n'),
        ('edit/long-line-edit', b'x' * 126 + b'\x08\x08y\x12\n'),
        ('edit/exactly-127', b'd' * 127 + b'\n'),
        ('edit/backspace-over-tab', b'\tab\x08\x08\x08\x08dir\n'),
        ('edit/ctl-x-tab', b'\tx\tx\x18dir\n'),
        ('edit/ctl-u-tab', b'\tx\x15dir\n'),
        ('edit/ctl-s-q', b'di\x13r\n'),
        ('edit/high-bit', b'di\xf2\n'),
        ('edit/nul-bytes', b'di\x00r\n'),
        ('edit/crlf-line', b'dir\r\n'),
        ('edit/ctl-z', b'di\x1ar\n'),
    ]
    for name, sc in ed:
        cases.append(Case('ccp/' + name, sc + b'\ndir\n', D))
    cases.append(Case('ccp/typeahead-ctl-c', S('type beta.txt\n', '\x03',
                                                'dir\n'), D, ['-t']))
    cases.append(Case('ccp/typeahead-ctl-s-c', S('type beta.txt\n', '\x13\x03',
                                                  'dir\n'), D, ['-t']))
    cases.append(Case('ccp/typeahead-ctl-s-x', S('type beta.txt\n', '\x13x',
                                                  'dir\n'), D, ['-t']))
    cases.append(Case('ccp/ctl-c-everywhere',
                      S('\x03', 'dir\n', 'era *.*\n', '\x03', 'dir\n'), D))

    # --- submit ---
    def sub(lines):
        recs = []
        for l in lines:
            recs.append((bytes([len(l)]) + l.encode().upper()).ljust(128, b'\0'))
        return b''.join(reversed(recs))
    sa = make_disk(0, a0 + [(0, '$$$.SUB', sub(['dir', 'type alpha.txt',
                                                'hello 1 2', 'era alpha.txt',
                                                'dir']))])
    cases.append(Case('submit/run', S('dir\n'), {'A': sa, 'B': b, 'C': c}))
    cases.append(Case('submit/break', S('x'), {'A': sa, 'B': b, 'C': c},
                      ['-t']))
    sb = make_disk(0, a0 + [(0, '$$$.SUB', sub(['dir', 'nosuchcmd', 'dir']))])
    cases.append(Case('submit/error', S('dir\n'), {'A': sb, 'B': b, 'C': c}))
    sc_ = make_disk(0, a0 + [(1, '$$$.SUB', sub(['dir']))])
    cases.append(Case('submit/other-user', S('user 1\n', 'dir\n', 'user 0\n'),
                      {'A': sc_, 'B': b, 'C': c}))
    cases.append(Case('submit/on-b', S('b:\n', 'dir\n', 'a:\n'),
                      {'A': sa, 'B': b, 'C': c}))
    return cases


# ---------------------------------------------------------------------------
# BDOS error handling cases

def error_cases():
    hello, _ = asm('hello')
    t1 = text(30, b'alpha')
    a = make_disk(0, [(0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1)])
    D = {'A': a, 'B': make_disk(1, [(0, 'B.TXT', t1)]), 'C': make_disk(2, [])}

    def S(*lines):
        return b''.join(l if isinstance(l, bytes) else l.encode()
                        for l in lines)
    cases = [
        Case('err/bad-sector-dir-x', S('dir\n', 'x', 'dir\n'), D, ['-e', '2']),
        Case('err/bad-sector-dir-ctlc', S('dir\n', '\x03', 'dir\n'), D,
             ['-e', '2']),
        Case('err/bad-sector-data', S('type alpha.txt\n', 'x', 'dir\n'),
             D, ['-e', '3']),
        Case('err/bad-sector-data-ctlc', S('type alpha.txt\n', '\x03',
                                           'dir\n'), D, ['-e', '3']),
        Case('err/bad-sector-write', S('save 1 x.dat\n', 'x', 'dir\n'), D,
             ['-e', '3']),
        Case('err/bad-sector-load', S('hello\n', 'x', 'dir\n'), D, ['-e', '3']),
        Case('err/bad-sector-b', S('b:\n', 'dir\n', 'x', 'type b.txt\n', 'x'),
             D, ['-e', '2']),
        Case('err/bad-sector-0', S('dir\n', 'x', 'dir\n'), D, ['-e', '0']),
        Case('err/select-d', S('d:\n', 'x', 'dir\n'), D),
        Case('err/select-d-dir', S('dir d:\n', '\x03', 'dir\n'), D),
        Case('err/select-d-type', S('type d:x.y\n', 'q', 'dir\n'), D),
        Case('err/select-unmounted-a', S('dir\n', 'x', 'dir\n'),
             {'A': None, 'B': D['B']}),
    ]
    return cases


# ---------------------------------------------------------------------------
# BDOS call fuzzing

FCB_BASE = 0x9000
SLOTS = 8


def fcb_bytes(drive, name11, tail=b''):
    return (bytes([drive]) + name11 + bytes(24)).ljust(36, b'\0')[:36]


def gen_fuzz(seed, ncalls, wild=True):
    r = random.Random(seed)
    names = [b'FILEA   DAT', b'FILEB   DAT', b'DATA    TXT', b'BIG     DAT',
             b'TMP     $$$', b'PROG    COM', b'ALPHA   TXT', b'HELLO   COM',
             b'X       Y  ', b'SPARSE  DAT']
    out = bytearray()
    log = []

    def call(func, de=0, setup=b'', dump=False):
        out.extend(tcall(func, de, setup, dump))

    def slot_addr(i):
        return FCB_BASE + i * 64

    open_slots = {}
    users = 0
    for n in range(ncalls):
        k = r.random()
        s = r.randrange(SLOTS)
        addr = slot_addr(s)
        drv = r.choice([0, 0, 0, 1, 2, 3])        # default, A, B, C
        if s in open_slots and k < 0.60:
            what = r.choice(['w'] * 8 + ['r'] * 5 + ['rr', 'rw', 'rz', 'close',
                                                      'size', 'setrand',
                                                      'rwfar', 'rseq'])
            if what == 'w':
                call(21, addr, dump=True)
            elif what == 'r':
                call(20, addr, dump=True)
            elif what == 'rseq':
                for _ in range(r.randrange(2, 6)):
                    call(20, addr, dump=True)
            elif what in ('rr', 'rw', 'rz', 'rwfar'):
                if what == 'rwfar':
                    rec = r.randrange(0, 4000)
                else:
                    rec = r.randrange(0, 700)
                if r.random() < 0.03:
                    rec = r.randrange(0x10000, 0x30000)
                setup = bytes([rec & 255, (rec >> 8) & 255, rec >> 16])
                func = {'rr': 33, 'rw': 34, 'rz': 40, 'rwfar': 34}[what]
                # poke r0..r2 using a harmless call (BDOS 12) as the carrier
                out.extend(tcall(12, addr + 33, setup))
                call(func, addr, dump=True)
            elif what == 'close':
                call(16, addr, dump=True)
                if r.random() < 0.7:
                    open_slots.pop(s, None)
            elif what == 'size':
                call(35, addr, dump=True)
            elif what == 'setrand':
                call(36, addr, dump=True)
            continue
        if k < 0.72:
            nm = r.choice(names)
            fc = fcb_bytes(drv, nm)
            call(22 if r.random() < 0.5 else 15, addr, fc, dump=True)
            if r.random() < 0.9:
                open_slots[s] = nm
        elif k < 0.76:
            nm = r.choice(names)
            if wild and r.random() < 0.4:
                nm = bytes(63 if r.random() < 0.3 else c for c in nm)
            call(19, addr, fcb_bytes(drv, nm), dump=True)
            open_slots.pop(s, None)
        elif k < 0.80:
            old = r.choice(names)
            new = r.choice(names)
            fc = bytearray(fcb_bytes(drv, old))
            fc[16:27] = new
            call(23, addr, bytes(fc), dump=True)
            open_slots.pop(s, None)
        elif k < 0.85:
            nm = r.choice(names)
            if wild and r.random() < 0.7:
                nm = bytes(63 if r.random() < 0.4 else c for c in nm)
            call(17, addr, fcb_bytes(drv, nm), dump=True)
            for _ in range(r.randrange(0, 5)):
                call(18, addr, dump=True)
            open_slots.pop(s, None)
        elif k < 0.87:
            nm = bytearray(r.choice(names))
            if r.random() < 0.5:
                nm[10] |= 0x80                       # SYS
            if r.random() < 0.3:
                nm[1] |= 0x80                        # f2' attribute
            call(30, addr, fcb_bytes(drv, bytes(nm)), dump=True)
            open_slots.pop(s, None)
        elif k < 0.89:
            u = r.choice([0, 0, 1, 2, 3, 0xFF])
            call(32, u)
            if u != 0xFF:
                users = u
        elif k < 0.91:
            call(14, r.choice([0, 1, 2]))
            call(25)
        elif k < 0.92:
            call(13)
            open_slots.clear()
        elif k < 0.93:
            f = r.choice([12, 24, 27, 29, 31, 7, 38, 39, 3, 11])
            call(f)
            if r.random() < 0.3:
                call(r.choice([4, 5, 8, 6]), r.choice([0x41, 0x55, 0xFE, 0x20]))
        elif k < 0.94:
            call(37, r.choice([0, 1, 2, 3, 4, 7]))
        else:
            # search with explicit user number in the fcb
            nm = r.choice(names)
            fc = bytearray(fcb_bytes(drv, nm))
            call(17, addr, bytes(fc), dump=True)
            open_slots.pop(s, None)
    out.append(0xFF)
    return bytes(out)


def fuzz_cases(nseeds):
    hello, _ = asm('hello')
    com, syms = asm('fuzz')
    assert syms['table'] == 0x100 + len(com), (syms['table'], len(com))
    t1 = text(20, b'alpha')
    cases = []
    for seed in range(nseeds):
        table = gen_fuzz(seed, 400 + 150 * (seed % 5))
        assert 0x100 + len(com) + len(table) < FCB_BASE, len(table)
        prog = com + table
        files = [(0, 'FUZZ.COM', prog), (0, 'HELLO.COM', hello),
                 (0, 'ALPHA.TXT', t1), (0, 'FILEA.DAT', pattern(5000, seed)),
                 (1, 'DATA.TXT', t1), (0, 'BIG.DAT', pattern(30000, seed + 1))]
        A = make_disk(0, files)
        B = make_disk(1, [(0, 'DATA.TXT', t1), (0, 'BIG.DAT',
                                                pattern(50000, 7)),
                          (0, 'FILEB.DAT', pattern(12000, 8))])
        C = make_disk(2, [(0, 'DATA.TXT', t1), (0, 'FILEA.DAT',
                                                pattern(70000, 9))])
        cases.append(Case('fuzz/seed%d' % seed, b'fuzz\ndir\ndir b:\ndir c:\n',
                          {'A': A, 'B': B, 'C': C}, ['-m', '300000000']))
    return cases



# ---------------------------------------------------------------------------
# directed BDOS tests (table programs)

def tcall(func, de=0, setup=b'', dump=False, rep=1):
    return bytes([func, 1 if dump else 0, de & 255, de >> 8, len(setup),
                  rep]) + setup


def treps(func, de, n):
    out = b''
    while n > 0:
        k = min(n, 255)
        out += tcall(func, de, rep=k)
        n -= k
    return out


def tpoke(addr, data):
    return tcall(12, addr, data)            # BDOS 12 carries the setup bytes


def tfcb(slot, name11, drive=0, new=None):
    fc = bytearray(fcb_bytes(drive, name11))
    if new:
        fc[16:27] = new
    return tpoke(FCB_BASE + slot * 64, bytes(fc))


def taddr(slot):
    return FCB_BASE + slot * 64


def tprog(table):
    com, syms = asm('fuzz')
    return com + table + b'\xff'


def directed_cases():
    hello, _ = asm('hello')
    t1 = text(20, b'alpha')
    ro_file = pattern(3000, 5)
    cases = []

    def disks(extra_a=(), c_files=()):
        a = make_disk(0, [(0, 'HELLO.COM', hello), (0, 'FILEA.DAT',
                                                    pattern(5000, 1)),
                          (0, 'RO.DAT;ro', ro_file),
                          (0, 'SYS.DAT;sys', t1)] + list(extra_a))
        b = make_disk(1, [(0, 'HELLO.COM', hello), (0, 'DATA.TXT', t1)])
        c = make_disk(2, list(c_files))
        return {'A': a, 'B': b, 'C': c}

    # --- console and misc functions ---
    io = (tpoke(0x9400, b'Hi there$') + tcall(9, 0x9400) + tcall(1) +
          tcall(6, 0xFF) + tcall(6, 0xFE) + tcall(6, ord('X')) +
          tcall(2, ord('Y')) + tcall(2, 9) + tcall(7) + tcall(8, 0x55) +
          tcall(7) + tcall(3) + tcall(4, ord('P')) + tcall(5, ord('L')) +
          tcall(10, 0x9500, bytes([20]), True) +
          tcall(10, 0x9600, bytes([5]), True) + tcall(11) + tcall(38) +
          tcall(39) + tcall(41) + tcall(0x80) + tcall(6, 0xFF) +
          tcall(6, 0xFF) + tcall(0))
    feed = b'a' + b'b' + b'0123456789\n' + b'\x10zz\x10ab\x7f\x08cdefghij\n'
    for flags in ([], ['-t']):
        d = disks()
        d['A'] = make_disk(0, [(0, 'IO.COM', tprog(io)),
                               (0, 'HELLO.COM', hello)])
        cases.append(Case('bdos/io-functions%s' % ''.join(flags),
                          b'io\n' + feed + b'dir\n', d, flags))

    # --- write protected disk ---
    f0 = tfcb(0, b'FILEA   DAT') + tcall(15, taddr(0), dump=True)
    ro_tests = {
        'write': tcall(21, taddr(0), dump=True),
        'make': tfcb(1, b'NEWFILE TXT') + tcall(22, taddr(1), dump=True),
        'delete': tcall(19, taddr(0), dump=True),
        'rename': tfcb(1, b'FILEA   DAT', new=b'OTHER   DAT') +
        tcall(23, taddr(1), dump=True),
        'randwrite': tpoke(taddr(0) + 33, bytes([3, 0, 0])) +
        tcall(34, taddr(0), dump=True),
        'zerofill': tpoke(taddr(0) + 33, bytes([200, 0, 0])) +
        tcall(40, taddr(0), dump=True),
        'close': tcall(16, taddr(0), dump=True) + tcall(29) + tcall(24),
        'attrs': tfcb(1, b'FILEA   DAT') + tcall(30, taddr(1), dump=True),
        'reset': tcall(37, 1) + tcall(29) + tcall(21, taddr(0), dump=True),
    }
    for nm, body in ro_tests.items():
        prog = tprog(f0 + tcall(28) + tcall(29) + body + tcall(0))
        d = disks()
        d['A'] = make_disk(0, [(0, 'T.COM', prog), (0, 'FILEA.DAT',
                                                    pattern(5000, 1))])
        cases.append(Case('bdos/ro-disk-%s' % nm, b't\nx' + b'dir\n', d))

    # --- read only files ---
    r0 = tfcb(0, b'RO      DAT') + tcall(15, taddr(0), dump=True)
    rf_tests = {
        'write': tcall(21, taddr(0), dump=True),
        'delete': tcall(19, taddr(0), dump=True),
        'rename': tfcb(1, b'RO      DAT', new=b'OTHER   DAT') +
        tcall(23, taddr(1), dump=True),
        'randwrite': tpoke(taddr(0) + 33, bytes([1, 0, 0])) +
        tcall(34, taddr(0), dump=True),
        'read-ok': tcall(20, taddr(0), dump=True) + tcall(16, taddr(0)),
        'wildcard-delete': tfcb(1, b'R???????DAT') +
        tcall(19, taddr(1), dump=True),
    }
    for nm, body in rf_tests.items():
        prog = tprog(r0 + body + tcall(0))
        d = disks()
        d['A'] = make_disk(0, [(0, 'T.COM', prog),
                               (0, 'RO.DAT;ro', ro_file),
                               (0, 'OTHER.DAT', t1)])
        cases.append(Case('bdos/ro-file-%s' % nm, b't\nx' + b'dir\n', d))
    d = disks()
    cases.append(Case('ccp/ro-file-era', b'era ro.dat\nx' + b'dir\n', d))
    cases.append(Case('ccp/ro-file-ren', b'ren z.z=ro.dat\nx' + b'dir\n', d))
    cases.append(Case('ccp/ro-file-ren-to', b'ren ro.dat=sys.dat\nx' + b'dir\n',
                      d))
    cases.append(Case('ccp/ro-file-type', b'type ro.dat\n' + b'dir\n', d))
    cases.append(Case('ccp/sys-file-dir', b'dir\ndir sys.dat\ntype sys.dat\n'
                                          b'dir *.dat\nera sys.dat\ndir\n', d))

    # --- big files: module/extent crossing on drive B, disk and dir full ---
    big = (tfcb(0, b'HUGE    DAT', drive=2) +
           tcall(22, taddr(0), dump=True) +
           tcall(21, taddr(0), rep=255) * 17 + tcall(21, taddr(0), rep=65) +
           tcall(16, taddr(0), dump=True) + tcall(35, taddr(0), dump=True) +
           tfcb(0, b'HUGE    DAT', drive=2) +
           tcall(15, taddr(0), dump=True) +
           tcall(20, taddr(0), rep=255) * 17 + tcall(20, taddr(0), rep=65) + tcall(20, taddr(0), dump=True) +
           tpoke(taddr(0) + 33, bytes([0x40, 0x10, 0])) +
           tcall(33, taddr(0), dump=True) +
           tcall(35, taddr(0), dump=True) + tcall(36, taddr(0), dump=True) +
           tcall(0))
    d = disks()
    d['A'] = make_disk(0, [(0, 'BIG.COM', tprog(big))])
    cases.append(Case('bdos/big-file-modules',
                      b'big\ndir b:\ndir\n', d, ['-m', '900000000']))

    small = [(0, 'F%02d.DAT' % i, b'') for i in range(58)]
    full = (tfcb(0, b'FILL    DAT', drive=3) + tcall(22, taddr(0), dump=True) +
            tcall(21, taddr(0), rep=255) * 12 + tcall(21, taddr(0), rep=240) + tcall(16, taddr(0), dump=True) +
            tcall(0))
    d = disks(c_files=small)
    d['A'] = make_disk(0, [(0, 'FULL.COM', tprog(full))])
    cases.append(Case('bdos/dir-full-on-extend', b'full\ndir c:\n', d,
                      ['-m', '900000000']))
    d = disks()
    d['A'] = make_disk(0, [(0, 'FULL.COM', tprog(full))])
    cases.append(Case('bdos/disk-full-on-extend', b'full\ndir c:\n', d,
                      ['-m', '900000000']))

    # --- CCP: user-number parsing, odd characters, submit on another drive ---
    D = disks()
    for i, cmd in enumerate(['user 999\n', 'user 1x\n', 'user 256\n',
                             'user 1000000000000\n', 'user 12345678901\n',
                             'user 00000000015\n', 'user 15\n', 'user b:3\n',
                             'user 3 4\n', 'user *\n', 'user ?\n']):
        cases.append(Case('ccp/user-parse-%d' % i, cmd.encode() + b'dir\n', D))
    cases.append(Case('ccp/high-chars',
                      b'dir{\n' + b'dir~\n' + b'DIR\x7b\n' + b'a~z\n' +
                      b'type {.txt\n', D))
    cases.append(Case('ccp/com-bad-load', b'hello\n', {
        'A': make_disk(0, [(0, 'HELLO.COM', b'\0' * 57000)]),
        'B': D['B'], 'C': D['C']}))

    def sub(lines):
        recs = []
        for l in lines:
            recs.append((bytes([len(l)]) + l.encode().upper()).ljust(128, b'\0'))
        return b''.join(reversed(recs))
    sw = make_disk(0, [(0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1),
                       (0, '$$$.SUB', sub(['b:', 'hello x', 'dir', 'type a:alpha.txt']))])
    cases.append(Case('submit/warm-boot-on-b', b'dir\n',
                      {'A': sw, 'B': D['B'], 'C': D['C']}))
    cases.append(Case('submit/warm-boot-on-b-break', b'x',
                      {'A': sw, 'B': D['B'], 'C': D['C']}, ['-t']))
    return cases



def edge_cases():
    hello, _ = asm('hello')
    badser, _ = asm('badser')
    t1 = text(20, b'alpha')
    cases = []

    # files that end exactly on an extent boundary, read to the end
    exact = {'E16.DAT': pattern(16384, 1), 'E32.DAT': pattern(32768, 2),
             'E64.DAT': pattern(65536, 3)}
    rd = lambda nm, drv, n: (tfcb(0, nm, drive=drv) +
                             tcall(15, taddr(0), dump=True) +
                             treps(20, taddr(0), n) +
                             tcall(20, taddr(0), dump=True) +
                             tcall(20, taddr(0), dump=True) +
                             tcall(35, taddr(0), dump=True) + tcall(0))
    for drv, kind in ((1, 0), (2, 1), (3, 2)):
        files = [(0, k, v) for k, v in exact.items()]
        for nm, size in (('E16 ', 128), ('E32 ', 256), ('E64 ', 512)):
            prog = tprog(rd(nm.strip().encode().ljust(8) + b'DAT', drv, size))
            d = {'A': make_disk(0, [(0, 'RD.COM', prog)]),
                 'B': make_disk(1, files), 'C': make_disk(2, files)}
            if kind == 0:
                d['A'] = make_disk(0, [(0, 'RD.COM', prog)] + files)
                prog = tprog(rd(nm.strip().encode().ljust(8) + b'DAT', 0, size))
                d['A'] = make_disk(0, [(0, 'RD.COM', prog)] + files)
            cases.append(Case('edge/exact-%s-%s' % (nm.strip(), 'ABC'[kind]),
                              b'rd\n', d, ['-m', '400000000']))

    # close with a map that disagrees with the directory
    fa = pattern(5000, 4)
    for desc, patch in (('single', bytes([0x77])), ('zero', bytes([0])),
                        ('double', bytes([0x00, 0x77]))):
        for drv, kind in ((0, 0), (2, 1)):
            body = (tfcb(0, b'FILEA   DAT', drive=drv) +
                    tcall(15, taddr(0), dump=True) +
                    tpoke(taddr(0) + 16, patch) +
                    tpoke(taddr(0) + 14, bytes([0])) +
                    tcall(21, taddr(0), dump=True) +
                    tpoke(taddr(0) + 16, patch) +
                    tcall(16, taddr(0), dump=True) +
                    tcall(16, taddr(0), dump=True) + tcall(0))
            prog = tprog(body)
            d = {'A': make_disk(0, [(0, 'T.COM', prog), (0, 'FILEA.DAT', fa)]),
                 'B': make_disk(1, [(0, 'FILEA.DAT', fa)]),
                 'C': make_disk(2, [])}
            cases.append(Case('edge/close-merge-%s-%s' % (desc, 'AB'[kind]),
                              b't\ndir\ndir b:\n', d))

    # directory modified behind the BDOS's back
    d = {'A': make_disk(0, [(0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1)]),
         'B': make_disk(1, []), 'C': make_disk(2, [])}
    g = mkdisk.GEOMS[0]
    dk = mkdisk.Disk(g)
    for rec, tag in ((0, 'inside'), (15, 'beyond')):
        off = dk.rec_off(rec) + 3
        for n in (17, 18, 25):
            cases.append(Case('edge/dir-changed-%s-at-%d' % (tag, n),
                              b'dir\ndir\nera alpha.txt\nx' + b'dir\n', d,
                              ['-x', '%d:0:%d:90' % (n, off)]))

    # submit file that deletes itself
    def sub(lines):
        return b''.join(reversed([(bytes([len(l)]) + l.upper().encode())
                                  .ljust(128, b'\0') for l in lines]))
    d = {'A': make_disk(0, [(0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1),
                            (0, '$$$.SUB', sub(['era $$$.sub', 'dir',
                                                'dir']))]),
         'B': make_disk(1, []), 'C': make_disk(2, [])}
    cases.append(Case('submit/self-delete', b'dir\n', d))

    # more CCP parsing corners
    D = {'A': make_disk(0, [(0, 'HELLO.COM', hello), (0, 'ALPHA.TXT', t1)]),
         'B': make_disk(1, []), 'C': make_disk(2, [])}
    for i, cmd in enumerate(['user 260\n', 'user 270\n', 'user 280\n',
                             'user 00000000.015\n', 'user 1.5\n',
                             'ren a.b=c*.d\n', 'ren a.b=c?.d\n',
                             'user 12345678.901\n']):
        cases.append(Case('ccp/parse-corner-%d' % i, cmd.encode() + b'dir\n', D))

    # serial number check
    d = {'A': make_disk(0, [(0, 'BADSER.COM', badser), (0, 'HELLO.COM', hello)]),
         'B': make_disk(1, []), 'C': make_disk(2, [])}
    cases.append(Case('ccp/bad-serial', b'badser\nhello\ndir\n', d, rc=4,
                      only=['port']))
    cases.append(Case('ccp/bad-serial-intrinsic', b'badser\ndir\nhello\n', d,
                      rc=4, only=['port']))
    return cases


# ---------------------------------------------------------------------------

def main(argv):
    verbose = '-v' in argv
    sub = None
    nseeds = 12
    if '-k' in argv:
        sub = argv[argv.index('-k') + 1]
    if '--fuzz-seeds' in argv:
        nseeds = int(argv[argv.index('--fuzz-seeds') + 1])
    global TARGETS
    if '--targets' in argv:
        TARGETS = argv[argv.index('--targets') + 1].split(',')
    if not os.path.exists(CPM) or not os.path.exists(REF + '/bdos.bin'):
        sys.exit('run "make" first')
    if 'z80' in TARGETS and not os.path.exists(Z80BIN):
        sys.exit('run "make z80" first')
    if 'i386' in TARGETS and not os.path.exists(I386ELF):
        sys.exit('run "make i386" first')
    cases = ccp_cases() + error_cases() + directed_cases() + edge_cases() + fuzz_cases(nseeds)
    if sub:
        cases = [c for c in cases if sub in c.name]
    ok = 0
    for c in cases:
        if check(c, verbose):
            ok += 1
    print('%d/%d passed' % (ok, len(cases)))
    sys.exit(0 if ok == len(cases) else 1)


if __name__ == '__main__':
    main(sys.argv[1:])
