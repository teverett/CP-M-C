/*
 * bdos.c - CP/M 2.2 BDOS, translated routine by routine from bdos.asm.
 *
 * Conventions used in the translation:
 *   - A routine that ended in "rz/rnz/rc/rnc" with a meaningful flag now
 *     returns that condition as a C value; the comment says which.
 *   - 8080 register pairs are plain uint16_t locals; 8-bit arithmetic is
 *     done in uint8_t so wrap-around matches the original.
 *   - "jmp reboot" is reboot(), which calls the BIOS warm boot and never
 *     returns.
 *   - Everything the BIOS, CCP or user programs can see (FCBs, DPH, DPB
 *     copies, allocation vectors) is addressed through mem[].
 *
 * Original: Copyright (c) 1978, 1979, 1980 Digital Research.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpm.h"

const cpm_bios *bios;
int bdos_trace;

#define ON_TRUE   0xFF
#define ON_FALSE  0x00

/* equates for non graphic characters */
#define CTLC   0x03
#define CTLE   0x05
#define CTLH   0x08
#define CTLP   0x10
#define CTLR   0x12
#define CTLS   0x13
#define CTLU   0x15
#define CTLX   0x18
#define CTLZ   0x1A
#define RUBOUT 0x7F
#define TAB    0x09
#define CR     0x0D
#define LF     0x0A
#define CTL    0x5E

#define SSIZE  24
#define DVERS  0x22
#define NFUNCS 41
#define ENDDIR 0xFFFF

/* fcb constants */
#define EMPTY  0xE5
#define LSTREC 127
#define RECSIZ 128
#define FCBLEN 32
#define DIRREC (RECSIZ / FCBLEN)
#define DSKSHF 2
#define DSKMSK (DIRREC - 1)
#define FCBSHF 5

#define EXTNUM 12
#define MAXEXT 31
#define UBYTES 13
#define MODNUM 14
#define MAXMOD 15
#define FWFMSK 0x80
#define NAMLEN 15
#define RECCNT 15
#define DSKMAP 16
#define LSTFCB (FCBLEN - 1)
#define NXTREC FCBLEN
#define RANREC (NXTREC + 1)

#define ROFILE 9

/* ------------------------------------------------------------------ */
/* data areas                                                          */

/* basic i/o */
static uint8_t compcol;      /* true if computing column position */
static uint8_t strtcol;      /* starting column position after read */
static uint8_t column;       /* column position */
static uint8_t listcp;       /* listing toggle */
static uint8_t kbchar;       /* initial key char = 00 */

/* common values */
static uint8_t usrcode;      /* current user number */
static uint8_t curdsk;       /* current disk number */
static uint16_t info;        /* information address */
static uint16_t aret;        /* address value to return */
#define lret_get()   ((uint8_t)aret)
#define lret_set(v)  (aret = (uint16_t)((aret & 0xFF00) | (uint8_t)(v)))

/* initialized data */
static uint16_t rodsk;       /* read only disk vector */
static uint16_t dlog;        /* logged-in disks */
static uint16_t dmaad;       /* dma address */

/* curtrka - alloca are set upon disk select */
static uint16_t cdrmaxa;     /* pointer to cur dir max value */
static uint16_t curtrka;     /* current track address */
static uint16_t curreca;     /* current record address */
static uint16_t buffa;       /* pointer to directory dma address */
static uint16_t dpbaddr;     /* current disk parameter block address */
static uint16_t checka;      /* current checksum vector address */
static uint16_t alloca_;     /* current allocation vector address */

/* sectpt - offset obtained from disk parm block at dpbaddr */
static uint16_t sectpt;      /* sectors per track */
static uint8_t  blkshf;      /* block shift factor */
static uint8_t  blkmsk;      /* block mask */
static uint8_t  extmsk;      /* extent mask */
static uint16_t maxall;      /* maximum allocation number */
static uint16_t dirmax;      /* largest directory number */
static uint16_t dirblk;      /* reserved allocation bits for directory */
static uint16_t chksiz;      /* size of checksum vector */
static uint16_t offset;      /* offset tracks at beginning */

/* local variables */
static uint16_t tranv;       /* address of translate vector */
static uint8_t  fcb_copied;  /* set true if copy_fcb called */
static uint8_t  rmf;         /* read mode flag for open_reel */
static uint8_t  dirloc;      /* directory flag in rename, etc. */
static uint8_t  seqio;       /* 1 if sequential i/o */
static uint8_t  linfo;       /* low(info) */
static uint8_t  dminx;       /* local for diskwrite */
static uint8_t  searchl;     /* search length */
static uint16_t searcha;     /* search address */
static uint8_t  single;      /* set true if single byte allocation map */
static uint8_t  resel;       /* reselection flag */
static uint8_t  olddsk;      /* disk on entry to bdos */
static uint8_t  fcbdsk;      /* disk named in fcb */
static uint8_t  rcount;      /* record count in current fcb */
static uint8_t  extval;      /* extent number and extmsk */
static uint8_t  vrecord;     /* current virtual record (low byte used) */
static uint16_t arecord;     /* current actual record */
static uint16_t arecord1;    /* current actual block# * blkmsk */

/* local variables for directory access */
static uint8_t  dptr;        /* directory pointer 0,1,2,3 */
static uint16_t dcnt;        /* directory counter 0,1,...,dirmax */
static uint16_t drec;        /* directory record 0,1,...,dirmax/4 */

static void persub(void);
static void selsub(void);
static void rodsub(void);
static void rofsub(void);

bdos_errfn pererr = persub;
bdos_errfn selerr = selsub;
bdos_errfn roderr = rodsub;
bdos_errfn roferr = rofsub;

void bdos_init(void)
{
    compcol = strtcol = column = listcp = kbchar = 0;
    usrcode = curdsk = 0;
    info = aret = 0;
    rodsk = dlog = 0;
    dmaad = TBUFF;
    cdrmaxa = curtrka = curreca = buffa = dpbaddr = checka = alloca_ = 0;
    sectpt = blkshf = blkmsk = extmsk = maxall = dirmax = dirblk = 0;
    chksiz = offset = 0;
    tranv = 0;
    fcb_copied = rmf = dirloc = seqio = linfo = dminx = searchl = 0;
    searcha = 0;
    single = resel = olddsk = fcbdsk = rcount = extval = vrecord = 0;
    arecord = arecord1 = 0;
    dptr = 0;
    dcnt = drec = 0;
    pererr = persub;
    selerr = selsub;
    roderr = rodsub;
    roferr = rofsub;
    wb(BDOS_EFCB, EMPTY);
}

/* jmp reboot */
static void reboot(void) __attribute__((noreturn));
static void reboot(void)
{
    bios->wboot();
    abort();                     /* wboot must not return */
}

/* ------------------------------------------------------------------ */
/* console handlers                                                    */

static void conout(uint8_t c);
static void tabout(uint8_t c);

static uint8_t conin(void)
{
    /* read console character to A */
    uint8_t a = kbchar;
    kbchar = 0;
    if (a != 0)
        return a;
    return bios->conin();
}

/* echoc: returns carry (true = not graphic and not cr/lf/tab/backspace) */
static int echoc(uint8_t a)
{
    if (a == CR || a == LF || a == TAB || a == CTLH)
        return 0;
    return a < ' ';
}

static uint8_t conech(void)
{
    /* read character with echo */
    uint8_t a = conin();
    if (echoc(a))
        return a;
    tabout(a);
    return a;
}

static uint8_t conbrk(void)
{
    /* check for character ready; returns A */
    uint8_t a;
    if (kbchar != 0)
        return 1;
    if ((bios->consts() & 1) == 0)
        return 0;
    a = bios->conin();
    if (a == CTLS) {
        a = bios->conin();
        if (a == CTLC)
            reboot();
        return 0;
    }
    kbchar = a;
    return 1;
}

static void conout(uint8_t c)
{
    /* compute character position/write console char from C */
    if (compcol == 0) {
        conbrk();
        bios->conout(c);
        if (listcp != 0)
            bios->list(c);
    }
    /* compout: compute column position */
    if (c == RUBOUT)
        return;
    column++;
    if (c >= ' ')
        return;
    column--;
    if (column == 0)
        return;
    if (c == CTLH) {
        column--;
        return;
    }
    if (c != LF)
        return;
    column = 0;
}

static void ctlout(uint8_t c)
{
    /* send C character with possible preceding up-arrow */
    if (!echoc(c)) {
        tabout(c);
        return;
    }
    conout(CTL);
    tabout((uint8_t)(c | 0x40));
}

static void tabout(uint8_t c)
{
    /* expand tabs to console */
    if (c != TAB) {
        conout(c);
        return;
    }
    do {
        conout(' ');
    } while ((column & 7) != 0);
}

static void pctlh(void)
{
    /* send ctlh to console without affecting column count */
    bios->conout(CTLH);
}

static void backup(void)
{
    /* back-up one screen position */
    pctlh();
    bios->conout(' ');
    pctlh();
}

static void crlf(void)
{
    conout(CR);
    conout(LF);
}

static void crlfp(void)
{
    /* print #, cr, lf for ctlx, ctlu, ctlr functions,
       then move to strtcol (starting column) */
    conout('#');
    crlf();
    while (column < strtcol)
        conout(' ');
}

static void print(uint16_t bc)
{
    /* print message until M(BC) = '$' */
    for (;;) {
        uint8_t a = rb(bc);
        if (a == '$')
            return;
        bc++;
        tabout(a);
    }
}

/* Same, for the C-string messages the BDOS itself prints */
static void print_str(const char *s)
{
    while (*s)
        tabout((uint8_t)*s++);
}

static void read_line(void)
{
    /* read to info address (max length, current length, buffer) */
    uint8_t a, b, c;
    uint16_t hl, lenaddr;

read:
    strtcol = column;
    hl = (uint16_t)(info + 1);
    lenaddr = hl;
    c = rb(info);                /* C = maximum buffer length */
    b = 0;                       /* B = current buffer length */
    /* HL = next to fill - 1 */
readnx:
readn0:
    a = conin() & 0x7F;          /* mask parity bit */
    if (a == CR || a == LF)
        goto readen;
    if (a == CTLH) {
        /* do we have any characters to back over? */
        if (b == 0)
            goto readnx;
        b--;
        compcol = column;        /* compcol > 0 marks repeat as length compute */
        goto linelen;
    }
    if (a == RUBOUT) {
        if (b == 0)
            goto readnx;
        a = rb(hl);              /* A = last char */
        b--;
        hl--;
        goto rdech1;
    }
    if (a == CTLE) {
        /* physical end line */
        crlf();
        strtcol = 0;
        goto readn0;
    }
    if (a == CTLP) {
        /* list toggle - change parity */
        listcp = (uint8_t)(1 - listcp);
        goto readnx;
    }
    if (a == CTLX) {
        /* backx: loop while column > strtcol */
        while (strtcol < column) {
            column--;
            backup();
        }
        goto read;
    }
    if (a == CTLU) {
        /* delete line */
        crlfp();
        goto read;
    }
    if (a != CTLR) {
        /* not a ctlr, place into buffer */
        hl++;
        wb(hl, a);
        b++;
        goto rdech1;
    }
linelen:
    {
        /* repeat line, or compute line len (ctlh) if compcol > 0 */
        uint16_t p = lenaddr;
        uint8_t n = b;
        crlfp();
        while (n != 0) {
            p++;
            n--;
            ctlout(rb(p));
        }
        hl = p;
        if (compcol == 0)
            goto readn0;
        /* column position computed for ctlh: move back compcol-column */
        compcol = (uint8_t)(compcol - column);
        do {
            backup();
            compcol--;
        } while (compcol != 0);
        goto readn0;
    }
rdech1:
    /* look for a random control character */
    ctlout(a);
    a = rb(hl);
    if (a == CTLC && b == 1)
        reboot();
    /* not reboot, are we at end of buffer? */
    if (b < c)
        goto readnx;
readen:
    /* end of read operation, store blen */
    wb(lenaddr, b);
    conout(CR);
}

/* ------------------------------------------------------------------ */
/* function results                                                    */

static void sta_ret(uint8_t a)
{
    /* store the A register to aret */
    lret_set(a);
}

static void setlret1(void)
{
    sta_ret(1);
}

static void sthl_ret(uint16_t hl)
{
    aret = hl;
}

/* ------------------------------------------------------------------ */
/* error handling                                                      */

static uint8_t errflg(const char *msg)
{
    /* report error to console, message in msg; returns console char */
    char dsk[2];
    crlf();
    print_str("Bdos Err On ");
    dsk[0] = (char)('A' + curdsk);
    dsk[1] = 0;
    print_str(dsk);
    print_str(": ");
    print_str(msg);
    return conin();
}

static void persub(void)
{
    /* report permanent error */
    uint8_t a = errflg("Bad Sector");
    if (a == CTLC)
        reboot();
    /* and ignore the error */
}

static void wait_err(const char *msg)
{
    /* wait for response before boot */
    errflg(msg);
    reboot();
}

static void selsub(void) { wait_err("Select"); }
static void rodsub(void) { wait_err("R/O"); }
static void rofsub(void) { wait_err("File R/O"); }

static void goerr(bdos_errfn *handler)
{
    /* HL = .errorhandler, call subroutine */
    (*handler)();
}

static void sel_error(void)
{
    goerr(&selerr);
}

/* ------------------------------------------------------------------ */
/* local subroutines for bios interface                                */

static void move(uint16_t de, uint16_t hl, uint8_t c)
{
    /* move data length of length C from source DE to destination HL */
    while (c != 0) {
        wb(hl, rb(de));
        de++;
        hl++;
        c--;
    }
}

/* Z flag = error.  Returns true when the disk was selected. */
static int selectdisk(uint8_t e)
{
    uint16_t hl, de;
    /* lsb of e = 0 if not yet logged - in */
    hl = bios->seldsk(curdsk, e);
    if (hl == 0)
        return 0;
    /* disk header block address in hl */
    de = rw(hl);                 /* DE=.tran */
    hl += 2;
    cdrmaxa = hl;
    hl += 2;
    curtrka = hl;
    hl += 2;
    curreca = hl;
    hl += 2;
    tranv = de;
    /* copy buffa, dpbaddr, checka, alloca (addlist = 8 bytes) */
    buffa    = rw(hl);
    dpbaddr  = rw((uint16_t)(hl + 2));
    checka   = rw((uint16_t)(hl + 4));
    alloca_  = rw((uint16_t)(hl + 6));
    /* now fill the disk parameter block (dpblist = 15 bytes) */
    {
        uint16_t d = dpbaddr;
        sectpt = rw(d);
        blkshf = rb((uint16_t)(d + 2));
        blkmsk = rb((uint16_t)(d + 3));
        extmsk = rb((uint16_t)(d + 4));
        maxall = rw((uint16_t)(d + 5));
        dirmax = rw((uint16_t)(d + 7));
        dirblk = rw((uint16_t)(d + 9));
        chksiz = rw((uint16_t)(d + 11));
        offset = rw((uint16_t)(d + 13));
    }
    /* now set single/double map mode */
    single = (maxall >> 8) == 0 ? ON_TRUE : ON_FALSE;
    return 1;
}

static void home(void)
{
    /* move to home position, then offset to start of dir */
    bios->home();
    ww(curtrka, 0);              /* curtrk = 0000 */
    ww(curreca, 0);              /* currec = 0000 */
}

static void diocomp(uint8_t a)
{
    /* check for disk errors */
    if (a == 0)
        return;
    goerr(&pererr);
}

static void rdbuff(void)
{
    /* read buffer and check condition */
    diocomp(bios->read());
}

static void wrbuff(uint8_t c)
{
    /* write buffer and check condition
       wrtype = 0 => normal write operation
       wrtype = 1 => directory write operation
       wrtype = 2 => start of new block */
    diocomp(bios->write(c));
}

static uint16_t hlrotr(uint16_t hl, uint8_t c)
{
    /* hl rotate right by amount C (logical) */
    while (c != 0) {
        hl >>= 1;
        c--;
    }
    return hl;
}

static void seek(void)
{
    /* seek the track given by arecord (actual record) */
    uint16_t bc = arecord;       /* arecord */
    uint16_t de = rw(curreca);   /* currec  */
    uint16_t hl = rw(curtrka);   /* curtrk  */
    uint32_t t;

    /* loop while arecord < currec */
    while (bc < de) {
        de = (uint16_t)(de - sectpt);   /* currec = currec - sectpt */
        hl--;                           /* curtrk = curtrk - 1 */
    }
    /* look while arecord >= (t:=currec + sectpt) */
    for (;;) {
        t = (uint32_t)sectpt + de;
        if (t > 0xFFFF)                 /* can be > FFFFH */
            break;
        if (bc < t)                     /* skip if t > arecord */
            break;
        de = (uint16_t)t;               /* currec = t */
        hl++;                           /* curtrk = curtrk + 1 */
    }
    bios->settrk((uint16_t)(offset + hl));
    ww(curtrka, hl);                    /* curtrk updated */
    ww(curreca, de);
    /* now compute sector as arecord-currec */
    bios->setsec(bios->sectran((uint16_t)(bc - de), tranv));
}

static void setdma(uint16_t hl_addr_of_var_value)
{
    bios->setdma(hl_addr_of_var_value);
}

static void setdata(void)  { setdma(dmaad); }   /* set data dma address */
static void setdir(void)   { setdma(buffa); }   /* set directory dma address */

static void seek_dir(void)
{
    /* seek the record containing the current dir entry */
    uint16_t hl = hlrotr(dcnt, DSKSHF);
    arecord = hl;
    drec = hl;
    seek();
}

/* ------------------------------------------------------------------ */
/* utility functions for file access                                   */

static uint8_t dm_position(void)
{
    /* compute disk map position for vrecord to A */
    uint8_t c = blkshf;
    uint8_t a = vrecord;
    uint8_t b;
    do {
        a >>= 1;
        c--;
    } while (c != 0);
    /* A = shr(vrecord,blkshf) */
    b = a;
    c = (uint8_t)(8 - blkshf);
    a = extval;
    for (;;) {
        c--;
        if (c == 0)
            break;
        a = (uint8_t)(a << 1);
    }
    return (uint8_t)(a + b);
}

static uint16_t getdm(uint16_t bc)
{
    /* return disk map value from position given by BC */
    uint16_t hl = (uint16_t)(info + DSKMAP + bc);
    if (single)
        return rb(hl);
    hl += bc;
    return rw(hl);
}

static void index_(void)
{
    /* compute disk block number from current fcb */
    uint8_t a = dm_position();
    arecord = getdm(a);
}

static int allocated(void)
{
    /* called following index to see if block allocated (true = allocated) */
    return arecord != 0;
}

static void atran(void)
{
    /* compute actual record address, assuming index called */
    uint8_t a = blkshf;
    uint16_t hl = arecord;
    uint8_t l;
    do {
        hl = (uint16_t)(hl << 1);
        a--;
    } while (a != 0);
    arecord1 = hl;               /* save shifted block # */
    l = (uint8_t)((vrecord & blkmsk) | (hl & 0xFF));
    arecord = (uint16_t)((hl & 0xFF00) | l);
}

static uint16_t getexta(void)
{
    /* get current extent field address */
    return (uint16_t)(info + EXTNUM);
}

static void getfcb(void)
{
    /* set variables from currently addressed fcb */
    vrecord = rb((uint16_t)(info + NXTREC));
    rcount  = rb((uint16_t)(info + RECCNT));
    extval  = (uint8_t)(extmsk & rb(getexta()));
}

static void setfcb(void)
{
    /* place values back into current fcb */
    uint8_t c = seqio;
    if (c == 2)                  /* check ranfill */
        c = 0;
    wb((uint16_t)(info + NXTREC), (uint8_t)(vrecord + c));
    wb((uint16_t)(info + RECCNT), rcount);
}

static uint8_t compute_cs(void)
{
    /* compute checksum for current directory buffer */
    uint8_t a = 0;
    uint16_t hl = buffa;
    uint8_t c = RECSIZ;
    do {
        a = (uint8_t)(a + rb(hl));
        hl++;
        c--;
    } while (c != 0);
    return a;
}

static uint16_t hlrotl(uint16_t hl, uint8_t c)
{
    /* rotate the mask in HL by amount in C */
    while (c != 0) {
        hl = (uint16_t)(hl << 1);
        c--;
    }
    return hl;
}

static uint16_t set_cdisk(uint16_t bc)
{
    /* set a "1" value in curdsk position of BC */
    return (uint16_t)(bc | hlrotl(1, curdsk));
}

static uint8_t nowrite(void)
{
    /* return true if dir checksum difference occurred */
    return (uint8_t)(hlrotr(rodsk, curdsk) & 1);
}

static void set_ro(void)
{
    /* set current disk to read only */
    rodsk = set_cdisk(rodsk);
    /* high water mark in directory goes to max */
    ww(cdrmaxa, (uint16_t)(dirmax + 1));
}

static void check_rofile(uint16_t hl)
{
    /* check current buff(dptr) or fcb(0) for r/o status */
    if ((rb((uint16_t)(hl + ROFILE)) & 0x80) == 0)
        return;
    goerr(&roferr);
}

static uint16_t getdptra(void)
{
    /* compute the address of a directory element at position dptr */
    return (uint16_t)(buffa + dptr);
}

static void check_rodir(void)
{
    /* check current directory element for read/only status */
    check_rofile(getdptra());
}

static void check_write(void)
{
    /* check for write protected disk */
    if (nowrite() == 0)
        return;
    goerr(&roderr);
}

static uint16_t getmodnum_addr(void)
{
    return (uint16_t)(info + MODNUM);
}

static void clrmodnum(void)
{
    /* clear the module number field for user open/make */
    wb(getmodnum_addr(), 0);
}

static uint8_t setfwf(void)
{
    /* set fwf (file write flag) to "1"; returns non zero */
    uint16_t hl = getmodnum_addr();
    uint8_t a = (uint8_t)(rb(hl) | FWFMSK);
    wb(hl, a);
    return a;
}

/* return carry if cdrmax > dcnt */
static int compcdr(void)
{
    return dcnt < rw(cdrmaxa);
}

static void setcdr(void)
{
    /* if not (cdrmax > dcnt) then cdrmax = dcnt+1 */
    if (compcdr())
        return;
    ww(cdrmaxa, (uint16_t)(dcnt + 1));
}

static void checksum(uint8_t c);

static void newchecksum(void)
{
    checksum(ON_TRUE);
}

static void checksum(uint8_t c)
{
    /* compute current checksum record and update the directory element
       if C=true, or check for = if not */
    uint8_t a;
    uint16_t hl;
    /* drec < chksiz? */
    if (!(drec < chksiz))
        return;                  /* skip checksum if past checksum vector size */
    a = compute_cs();
    hl = (uint16_t)(checka + drec);
    if ((uint8_t)(c + 1) == 0) {
        /* initializing the checksum */
        wb(hl, a);
        return;
    }
    /* not initializing, compare */
    if (a == rb(hl))
        return;                  /* no message if ok */
    /* checksum error, are we beyond the end of the disk? */
    if (!compcdr())
        return;
    set_ro();                    /* read/only disk set */
}

static void wrdir(void)
{
    /* write the current directory entry, set checksum */
    newchecksum();
    setdir();
    wrbuff(1);
    setdata();
}

static void rd_dir(void)
{
    /* read a directory entry into the directory buffer */
    setdir();
    rdbuff();
    setdata();                   /* falls through into setdata */
}

static void dir_to_user(void)
{
    /* copy the directory entry to the user buffer */
    move(buffa, dmaad, RECSIZ);
}

/* returns true (zero flag) if at end of directory */
static int end_of_dir(void)
{
    return dcnt == ENDDIR;
}

static void set_end_dir(void)
{
    dcnt = ENDDIR;
}

static void read_dir(uint8_t c)
{
    /* read next directory entry, with C=true if initializing */
    uint8_t a;
    dcnt++;
    /* continue while dirmax >= dcnt */
    if (dirmax < dcnt) {
        set_end_dir();
        return;
    }
    a = (uint8_t)((dcnt & DSKMSK) << FCBSHF);
    dptr = a;
    if (a != 0)
        return;                  /* return if not a new record */
    seek_dir();
    rd_dir();
    checksum(c);
}

/* getallocbit: returns the byte rotated so the bit is in bit 0;
   *hl = address of the byte, *d = number of rotations */
static uint8_t getallocbit(uint16_t bc, uint16_t *hl, uint8_t *d)
{
    uint8_t e = (uint8_t)((bc & 7) + 1);
    uint8_t a;
    *d = e;
    bc >>= 3;
    *hl = (uint16_t)(alloca_ + bc);
    a = rb(*hl);
    while (e != 0) {
        a = (uint8_t)((a << 1) | (a >> 7));      /* rlc */
        e--;
    }
    return a;
}

static void rotr(uint16_t hl, uint8_t a, uint8_t d)
{
    /* rotate back into position and replace */
    do {
        a = (uint8_t)((a >> 1) | (a << 7));      /* rrc */
        d--;
    } while (d != 0);
    wb(hl, a);
}

static void set_alloc_bit(uint16_t bc, uint8_t e)
{
    /* BC is the bit position of ALLOC to set or reset.
       The value of the bit is in register E. */
    uint16_t hl;
    uint8_t d;
    uint8_t a = getallocbit(bc, &hl, &d);
    a = (uint8_t)((a & 0xFE) | e);
    rotr(hl, a, d);
}

static void scandm(uint8_t c)
{
    /* scan the disk map addressed by dptr for non-zero entries; the
       allocation vector entry corresponding to a non-zero entry is set
       to the value of C (0,1) */
    uint16_t hl = (uint16_t)(getdptra() + DSKMAP);
    uint8_t cnt = FCBLEN - DSKMAP + 1;
    for (;;) {
        uint16_t bc;
        cnt--;                   /* all done scanning? */
        if (cnt == 0)
            return;
        if (single) {
            bc = rb(hl);
        } else {
            cnt--;               /* count for double byte */
            bc = rb(hl);
            hl++;
            bc |= (uint16_t)(rb(hl) << 8);
        }
        /* BC=block#, E=0/1 */
        if (bc != 0) {
            if (!(maxall < bc))  /* check invalid index */
                set_alloc_bit(bc, c);
        }
        hl++;                    /* to next bit position */
    }
}

static void initialize(void)
{
    /* initialize the current disk */
    uint16_t hl, bc, de;
    /* compute the length of the allocation vector - 2:
       number of bytes in alloc vector is (maxall/8)+1 */
    hl = hlrotr(maxall, 3);
    hl++;
    bc = hl;
    hl = alloca_;
    /* fill the allocation vector with zeros */
    do {
        wb(hl, 0);
        hl++;
        bc--;
    } while (bc != 0);
    /* set the reserved space for the directory */
    de = dirblk;
    ww(alloca_, de);
    /* allocation vector initialized, home disk */
    home();
    /* cdrmax = 3 (scans at least one directory record) */
    ww(cdrmaxa, 3);
    set_end_dir();               /* dcnt = enddir */
    /* read directory entries and check for allocated storage */
    for (;;) {
        read_dir(ON_TRUE);
        if (end_of_dir())
            return;
        /* not end of directory, valid entry? */
        hl = getdptra();
        if (rb(hl) == EMPTY)
            continue;
        /* not empty, user code the same? */
        if (usrcode == rb(hl)) {
            /* same user code, check for '$' submit */
            if (rb((uint16_t)(hl + 1)) == '$')
                lret_set(0xFF);  /* dollar file found, mark in lret */
        }
        /* now scan the disk map for allocated blocks */
        scandm(1);
        setcdr();                /* set cdrmax to dcnt */
    }
}

static void select_(void)
{
    /* select disk info for subsequent input or output ops */
    uint16_t hl = hlrotr(dlog, curdsk);
    if (!selectdisk((uint8_t)hl))
        sel_error();
    /* is the disk logged in? */
    if (hl & 1)
        return;
    /* disk not logged in, set bit and initialize */
    dlog = set_cdisk(dlog);
    initialize();
}

static void curselect(void)
{
    if (linfo == curdsk)
        return;
    curdsk = linfo;
    select_();
}

static void reselect(void)
{
    /* check current fcb to see if reselection necessary */
    uint8_t a;
    resel = ON_TRUE;             /* mark possible reselect */
    a = (uint8_t)((rb(info) & 0x1F) - 1);   /* drive code normalized 0..30, or 255 */
    linfo = a;
    if (a < 30) {
        /* auto select function, save curdsk */
        olddsk = curdsk;
        fcbdsk = rb(info);       /* save drive code */
        wb(info, (uint8_t)(rb(info) & 0xE0));   /* preserve hi bits */
        curselect();
    }
    /* set user code */
    wb(info, (uint8_t)(usrcode | rb(info)));
}

/* ------------------------------------------------------------------ */
/* directory search                                                    */

/* returns nonzero if the extents do not match */
static uint8_t compext(uint8_t a, uint8_t c)
{
    uint8_t b = (uint8_t)~extmsk;
    c &= b;
    a &= b;
    return (uint8_t)((uint8_t)(a - c) & MAXEXT);
}

static void searchn(void)
{
    /* search for the next directory element, assuming a previous call
       on search which sets searcha and searchl */
    uint16_t de, hl;
    uint8_t a, b, c;

again:
    read_dir(ON_FALSE);          /* read next dir element */
    if (end_of_dir())
        goto search_fin;
    /* not end of directory, scan for match */
    de = searcha;
    a = rb(de);                  /* first character */
    if (a != EMPTY) {
        /* not empty, may be end of logical directory */
        if (!compcdr())
            goto search_fin;     /* artificial stop */
    }
    hl = getdptra();
    c = searchl;                 /* length of search */
    b = 0;                       /* b counts up, c counts down */
    for (;;) {
        if (c == 0)
            goto endsearch;
        a = rb(de);
        if (a != '?') {                      /* ? matches all */
            if (b != UBYTES) {               /* ubytes field matches all */
                if (b == EXTNUM) {
                    /* attempt an extent # match */
                    if (compext(a, rb(hl)) != 0)
                        goto again;
                } else {
                    /* mask-out flags/extent modulus */
                    if (((a - rb(hl)) & 0x7F) != 0)
                        goto again;
                }
            }
        }
        /* searchok: current character matches */
        de++;
        hl++;
        b++;
        c--;
    }
endsearch:
    /* entire name matches, return dir position */
    lret_set(dcnt & DSKMSK);
    if (dirloc & 0x80)
        dirloc = 0;              /* change it to 0 to mark as found */
    return;
search_fin:
    /* end of directory, or empty name */
    set_end_dir();               /* may be artificial end */
    sta_ret(255);
}

static void search(uint8_t c)
{
    /* search for directory element of length C at info */
    dirloc = 0xFF;               /* changed if actually found */
    searchl = c;
    searcha = info;
    set_end_dir();               /* dcnt = enddir */
    home();                      /* to start at the beginning */
    searchn();
}

static void delete_(void)
{
    /* delete the currently addressed file */
    check_write();               /* write protected? */
    search(EXTNUM);              /* search through file type */
    /* loop while directory matches */
    while (!end_of_dir()) {
        /* set each non zero disk map entry to 0 in the allocation vector;
           may be r/o file */
        check_rodir();           /* ro disk error if found */
        wb(getdptra(), EMPTY);
        scandm(0);               /* alloc elts set to 0 */
        wrdir();                 /* write the directory */
        searchn();               /* to next element */
    }
}

static uint16_t get_block(uint16_t bc)
{
    /* given allocation vector position BC, find the zero bit closest to
       this position by searching left and right.  If found, set the bit
       to one and return the bit position.  If not found (i.e., we pass 0
       on the left, or maxall on the right), return 0000 */
    uint16_t de = bc;            /* copy of starting position to de */
    uint16_t hl;
    uint8_t a, d;

lefttst:
    if (bc != 0) {               /* skip if left=0000 */
        /* left not at position zero, bit zero? */
        bc--;
        a = getallocbit(bc, &hl, &d);
        if ((a & 1) == 0) {
            /* retblock: bit back into position and set to 1 */
            a = (uint8_t)((a & 0xFE) + 1);
            rotr(hl, a, d);
            return bc;           /* HL returned value */
        }
        /* bit is one, so try the right */
    }
    if (de >= maxall)            /* right=maxall? */
        goto retblock0;
    de++;
    {
        uint16_t addr;
        a = getallocbit(de, &addr, &d);
        if ((a & 1) == 0) {
            hl = de;
            /* retblock: bit back into position and set to 1 */
            a = (uint8_t)((a & 0xFE) + 1);
            rotr(addr, a, d);
            return hl;
        }
    }
    goto lefttst;               /* for another attempt */

retblock0:
    /* cannot find an available bit, return 0000 */
    if (bc != 0)                 /* also at beginning */
        goto lefttst;
    return 0;
}

static void seek_copy(void)
{
    /* enter from close to seek and copy current element */
    seek_dir();                  /* to the directory element */
    wrdir();                     /* write the directory element */
}

static void copy_dir(uint8_t c, uint8_t e)
{
    /* copy fcb information starting at C for E bytes into the
       currently addressed directory entry */
    move((uint16_t)(info + c), getdptra(), e);
    seek_copy();
}

static void copy_fcb(void)
{
    /* copy the entire file control block */
    copy_dir(0, FCBLEN);
}

static void rename_(void)
{
    /* rename the file described by the first half of the currently
       addressed file control block.  The new name is contained in the
       last half of the currently addressed file control block. */
    check_write();               /* may be write protected */
    search(EXTNUM);              /* search up to the extent field */
    /* copy position 0 */
    wb((uint16_t)(info + DSKMAP), rb(info));
    /* assume the same disk drive for new named file */
    while (!end_of_dir()) {
        check_rodir();           /* may be read-only file */
        copy_dir(DSKMAP, EXTNUM);
        searchn();
    }
}

static void indicators(void)
{
    /* set file indicators for current fcb */
    search(EXTNUM);              /* through file type */
    while (!end_of_dir()) {
        copy_dir(0, EXTNUM);     /* copy name */
        searchn();
    }
}

static void open_copy(void)
{
    /* copy fcb info from the directory element at dptr */
    uint16_t hl = getexta();
    uint8_t save_ext = rb(hl);   /* save extent# */
    uint16_t de = getdptra();
    uint8_t c, b, a;

    move(de, info, NXTREC);      /* entire fcb is copied, incl. indicators */
    setfwf();                    /* sets file write flag */
    c = rb((uint16_t)(de + EXTNUM));    /* C = directory extent number */
    b = rb((uint16_t)(de + RECCNT));    /* B holds directory record count */
    wb(hl, save_ext);            /* restore extent number */
    /* if user ext < dir ext then user := 128 records
       if user ext = dir ext then user := dir records
       if user ext > dir ext then user := 0 records */
    if (c == save_ext)
        a = b;                   /* if same, user gets dir reccnt */
    else if (c < save_ext)
        a = 0;                   /* user is larger */
    else
        a = 128;                 /* directory is larger */
    wb((uint16_t)(info + RECCNT), a);
}

static void open_(void)
{
    /* search for the directory entry, copy to fcb */
    search(NAMLEN);
    if (end_of_dir())
        return;                  /* return with lret=255 if end */
    open_copy();
}

static void mergezero(uint16_t hl, uint16_t de)
{
    /* HL = .fcb1(i), DE = .fcb2(i),
       if fcb1(i) = 0 then fcb1(i) := fcb2(i) */
    if (rw(hl) != 0)
        return;
    ww(hl, rw(de));
}

static void close_(void)
{
    /* locate the directory element and re-write it */
    uint16_t de, hl;
    uint8_t c, a, b;

    lret_set(0);
    dcnt = 0;
    if (nowrite())               /* skip close if r/o disk */
        return;
    /* check file write flag - 0 indicates written */
    if (rb(getmodnum_addr()) & FWFMSK)
        return;                  /* return if bit remains set */
    search(NAMLEN);              /* locate file */
    if (end_of_dir())
        return;                  /* return if not found */
    /* merge the disk map at info with that at buff(dptr) */
    de = (uint16_t)(getdptra() + DSKMAP);
    hl = (uint16_t)(info + DSKMAP);
    c = FCBLEN - DSKMAP;         /* length of single byte dm */
    for (;;) {
        if (single) {
            /* this is a single byte map:
               if fcb(i) = 0 then fcb(i) = buff(i)
               if buff(i) = 0 then buff(i) = fcb(i)
               if fcb(i) <> buff(i) then error */
            a = rb(hl);
            b = rb(de);
            if (a == 0)
                wb(hl, b);       /* fcb(i) = buff(i) */
            if (b == 0) {
                b = rb(hl);      /* buff(i) = fcb(i) */
                wb(de, b);
            }
            if (b != rb(hl))
                goto mergerr;
        } else {
            /* this is a double byte merge operation */
            mergezero(hl, de);   /* fcb = buff if fcb 0000 */
            mergezero(de, hl);   /* buff = fcb if buff 0000 */
            /* they should be identical at this point */
            if (rb(de) != rb(hl))
                goto mergerr;
            de++;                /* to high byte */
            hl++;
            if (rb(de) != rb(hl))
                goto mergerr;
            /* merge operation ok for this pair */
            c--;                 /* extra count for double byte */
        }
        /* dmset */
        de++;                    /* to next byte position */
        hl++;
        c--;
        if (c == 0)
            break;
    }
    /* end of disk map merge, check record count:
       DE = .fcb(extnum), HL = .buff(dptr+extnum) */
    {
        uint16_t fcb_ext = (uint16_t)(info + EXTNUM);
        uint16_t buf_ext = (uint16_t)(getdptra() + EXTNUM);
        a = rb(fcb_ext);
        /* if fcb(ext) >= buff(ext) then
           buff(ext) := fcb(ext), buff(rec) := fcb(rec) */
        if (!(a < rb(buf_ext))) {
            wb(buf_ext, a);
            wb((uint16_t)(buf_ext + (RECCNT - EXTNUM)),
               rb((uint16_t)(fcb_ext + (RECCNT - EXTNUM))));
        }
    }
    fcb_copied = ON_TRUE;        /* mark as copied */
    seek_copy();                 /* ok to "wrdir" here - 1.4 compat */
    return;
mergerr:
    /* elements did not merge correctly */
    lret_set((uint8_t)(lret_get() - 1));     /* =255 non zero flag set */
}

static void make_(void)
{
    /* create a new file by creating a directory entry
       then opening the file */
    uint16_t de, hl;
    uint8_t c;

    check_write();               /* may be write protected */
    de = info;                   /* save fcb address, look for e5 */
    info = BDOS_EFCB;            /* info = .empty */
    search(1);                   /* length 1 match on empty entry */
    {
        int none = end_of_dir(); /* zero flag set if no space */
        info = de;               /* in case we return here */
        if (none)
            return;              /* return with error condition 255 */
    }
    /* clear the remainder of the fcb */
    hl = (uint16_t)(de + NAMLEN);
    c = FCBLEN - NAMLEN;         /* number of bytes to fill */
    do {
        wb(hl, 0);
        hl++;
        c--;
    } while (c != 0);
    wb((uint16_t)(de + UBYTES), 0);          /* fcb(ubytes) = 0 */
    setcdr();                    /* may have extended the directory */
    /* now copy entry to the directory */
    copy_fcb();
    /* and set the file write flag to "1" */
    setfwf();
}

static void open_reel(void)
{
    /* close the current extent, and open the next one if possible.
       RMF is true if in read mode */
    uint16_t hl;
    uint8_t a;

    fcb_copied = 0;              /* set true if actually copied */
    close_();                    /* close current extent */
    /* lret remains at enddir if we cannot open the next ext */
    if (end_of_dir())
        return;
    /* increment extent number */
    hl = (uint16_t)(info + EXTNUM);
    a = (uint8_t)((rb(hl) + 1) & MAXEXT);
    wb(hl, a);
    if (a == 0)
        goto open_mod;           /* move to next module if zero */
    /* may be in the same extent group */
    a = (uint8_t)(extmsk & a);
    /* if result is zero, then not in the same group */
    a &= fcb_copied;             /* true if the fcb was copied to directory */
    if (a == 0)
        goto open_reel0;         /* go to next physical extent */
    /* result is non zero, so we must be in same logical ext */
    goto open_reel1;             /* to copy fcb information */
open_mod:
    /* extent number overflow, go to next module */
    hl = (uint16_t)(info + MODNUM);
    wb(hl, (uint8_t)(rb(hl) + 1));
    /* module number incremented, check for overflow */
    if ((rb(hl) & MAXMOD) == 0)
        goto open_r_err;         /* cannot overflow to zero */
open_reel0:
    search(NAMLEN);              /* next extent found? */
    if (!end_of_dir())
        goto open_reel1;
    /* end of file encountered */
    if ((uint8_t)(rmf + 1) == 0)
        goto open_r_err;         /* sets lret = 1 */
    /* try to extend the current file */
    make_();
    /* cannot be end of directory */
    if (end_of_dir())
        goto open_r_err;         /* with lret = 1 */
    goto open_reel2;
open_reel1:
    /* not end of file, open */
    open_copy();
open_reel2:
    getfcb();                    /* set parameters */
    sta_ret(0);                  /* lret = 0 */
    return;
open_r_err:
    /* cannot move to next extent of this file */
    setlret1();                  /* lret = 1 */
    setfwf();                    /* ensure that it will not be closed */
}

static void diskread(void)
{
    /* (may enter from seqdiskread) */
    rmf = ON_TRUE;               /* read mode flag = true (open_reel) */
    /* read the next record from the current fcb */
    getfcb();                    /* sets parameters for the read */
    if (!(vrecord < rcount)) {
        /* not enough records in the extent;
           record count must be 128 to continue */
        if (vrecord != 128)
            goto diskeof;
        open_reel();             /* go to next extent if so */
        vrecord = 0;
        /* now check for open ok */
        if (lret_get() != 0)
            goto diskeof;        /* stop at eof */
    }
    /* recordok: arrive with fcb addressing a record to read */
    index_();
    /* error 2 if reading unwritten data (returns 1 for 1.4 compat) */
    if (!allocated())
        goto diskeof;
    /* record has been allocated, read it */
    atran();                     /* arecord now a disk address */
    seek();                      /* to proper track,sector */
    rdbuff();                    /* to dma address */
    setfcb();                    /* replace parameter */
    return;
diskeof:
    setlret1();                  /* lret = 1 */
}

static void seqdiskread(void)
{
    /* sequential disk read operation */
    seqio = 1;
    diskread();
}

static void diskwrite(void)
{
    /* (may enter here from seqdiskwrite) */
    uint8_t a, c;
    uint16_t hl, bc;

    rmf = ON_FALSE;              /* read mode flag */
    /* write record to currently selected file */
    check_write();               /* in case write protected */
    check_rofile(info);          /* may be a read-only file */
    getfcb();                    /* to set local parameters */
    if (vrecord >= LSTREC + 1) {
        /* vrecord = 128, cannot open next extent */
        setlret1();              /* lret=1 */
        return;
    }
    /* can write the next record, so continue */
    index_();
    c = 0;                       /* marked as normal write operation */
    if (!allocated()) {
        /* not allocated.  The argument to getblock is the starting
           position for the disk search, and should be the last allocated
           block for this file, or the value 0 if no space has been
           allocated */
        a = dm_position();
        dminx = a;               /* save for later */
        bc = 0;                  /* may use block zero */
        if (a != 0) {
            /* previous block exists at A */
            bc = (uint16_t)(a - 1);
            bc = getdm(bc);      /* previous block # */
        }
        /* BC = 0000, or previous block # */
        hl = get_block(bc);      /* block # to HL */
        /* arrive here with block# or zero */
        if (hl == 0) {
            /* cannot find a block to allocate */
            sta_ret(2);          /* lret=2 */
            return;
        }
        /* allocated block number is in HL */
        arecord = hl;
        {
            uint16_t p = (uint16_t)(info + DSKMAP);
            if (single) {
                /* allocating a byte value */
                p = (uint16_t)(p + dminx);
                wb(p, (uint8_t)hl);
            } else {
                /* allocate a word value */
                p = (uint16_t)(p + 2 * dminx);
                ww(p, hl);
            }
        }
        /* disk write to previously unallocated block */
        c = 2;                   /* marked as unallocated write */
    }
    /* diskwr1: continue the write operation of no allocation error;
       C = 0 if normal write, 2 if to prev unalloc block */
    if (lret_get() != 0)
        return;                  /* stop if non zero returned value */
    atran();                     /* arecord set */
    if (seqio == 2 && c == 2) {
        /* zero fill of the new block (function 40) */
        uint16_t saved = arecord;
        uint8_t cc = 2;
        uint8_t i;
        uint16_t p;
        p = buffa;               /* zero buffa & fill */
        for (i = 0; i < 128; i++)
            wb((uint16_t)(p + i), 0);
        setdir();
        hl = arecord1;
        for (;;) {
            arecord = hl;
            seek();
            wrbuff(cc);          /* write fill record */
            hl = arecord;        /* restore last record */
            cc = 0;              /* change allocate flag */
            a = blkmsk;
            if (((uint8_t)hl & a) == a) {
                break;           /* until cluster is zeroed */
            }
            hl++;
        }
        arecord = saved;
        setdata();
    }
    /* diskwr11 */
    seek();                      /* to proper file position */
    wrbuff(c);                   /* written to disk (C=2 if new block) */
    /* increment record count if rcount<=vrecord */
    a = vrecord;
    if (!(a < rcount)) {
        /* rcount <= vrecord */
        rcount = (uint8_t)(a + 1);
        c = 2;                   /* mark as record count incremented */
    }
    /* patch1: always reset the file write flag to mark as written fcb */
    hl = getmodnum_addr();
    wb(hl, (uint8_t)(rb(hl) & ~FWFMSK));
    /* noupdate: check for end of extent, if found attempt to open next
       extent in preparation for next write */
    if (a == LSTREC && seqio == 1) {
        /* may be random access write, if so we are done */
        setfcb();                /* update current fcb before next extent */
        open_reel();             /* rmf=false */
        /* vrecord remains at lstrec causing eof if no more directory
           space is available */
        if (lret_get() == 0)
            vrecord = 0xFF;      /* space available, goes to 00 next time */
        lret_set(0);             /* lret = 00 for returned value */
    }
    setfcb();                    /* replace parameters */
}

static void seqdiskwrite(void)
{
    /* sequential disk write */
    seqio = 1;
    diskwrite();
}

/* rseek returns 0 (zero flag) on success, non zero on error */
static uint8_t rseek1(uint8_t cflag)
{
    uint16_t de, hl;
    uint8_t a, b, c, l, rec, b0, b1, b2;

    de = info;                   /* DE will hold base of fcb */
    hl = (uint16_t)(de + RANREC);
    b0 = rb(hl);
    b1 = rb((uint16_t)(hl + 1));
    b2 = rb((uint16_t)(hl + 2));
    rec = b0 & 0x7F;             /* sought record */
    c = (uint8_t)(((b1 << 1) | (b0 >> 7)) & 0x1F);  /* extent number */
    b = (uint8_t)((b1 >> 4) & 0x0F);                /* module number */
    /* check to insure that high byte of ran rec = 00 */
    l = 6;                       /* produce error 6, seek past physical eod */
    if (b2 != 0)
        goto seekerr;
    /* otherwise, high byte = 0, A = sought record */
    wb((uint16_t)(de + NXTREC), rec);        /* sought rec# stored away */
    /* arrive here with B=mod#, C=ext#, DE=.fcb, rec stored;
       the r/w flag is cflag.  compare fcb values */
    a = (uint8_t)(c - rb((uint16_t)(de + EXTNUM)));  /* tests for = extents */
    if (a == 0) {
        /* extents match, check mod#; could be overflow at eof, producing
           module# of 90H or 10H, so compare all but fwf */
        a = (uint8_t)((b - rb((uint16_t)(de + MODNUM))) & 0x7F);
        if (a == 0)
            goto seekok;
    }
    /* ranclose: current extent closed */
    close_();
    l = 3;                       /* cannot close error #3 */
    if ((uint8_t)(lret_get() + 1) == 0)
        goto badseek;
    wb((uint16_t)(de + EXTNUM), c);          /* fcb(extnum) = ext# */
    wb((uint16_t)(de + MODNUM), b);          /* fcb(modnum) = mod# */
    open_();                     /* is the file present? */
    if ((uint8_t)(lret_get() + 1) != 0)      /* open successful? */
        goto seekok;
    /* cannot open the file, read mode? */
    l = 4;                       /* seek to unwritten extent #4 */
    if ((uint8_t)(cflag + 1) == 0)
        goto badseek;            /* skip to error if read operation */
    /* write operation, make new extent */
    make_();
    l = 5;                       /* cannot create new extent #5 */
    if ((uint8_t)(lret_get() + 1) == 0)
        goto badseek;            /* no dir space */
    /* file make operation successful */
seekok:
    sta_ret(0);                  /* with zero set */
    return 0;
badseek:
    /* fcb no longer contains a valid fcb, mark with 1100_0000b in modnum
       field so that it appears as overflow with file write flag set */
    wb(getmodnum_addr(), 0xC0);
seekerr:
    lret_set(l);                 /* lret=#, nonzero */
    return setfwf();             /* flag set, so subsequent close ok */
}

static uint8_t rseek(uint8_t c)
{
    /* random access seek operation, C=0ffh if read mode */
    seqio = 0;                   /* marked as random access operation */
    return rseek1(c);
}

static void randiskread(void)
{
    /* random disk read operation */
    if (rseek(ON_TRUE) == 0)     /* marked as read operation */
        diskread();              /* if seek successful */
}

static void randiskwrite(void)
{
    /* random disk write operation */
    if (rseek(ON_FALSE) == 0)    /* marked as write operation */
        diskwrite();             /* if seek successful */
}

/* compute_rr: base = .buf(dptr) or .fcb(0), off = reccnt or nxtrec.
   Returns A = carry-out bit, *bc = mmmm eeee errr rrrr */
static uint8_t compute_rr(uint16_t base, uint8_t off, uint16_t *bc_out)
{
    uint8_t b, c, a, carry1, carry2;
    unsigned t;

    c = rb((uint16_t)(base + off));          /* BC = 0000 0000 ?rrr rrrr */
    b = 0;
    a = rb((uint16_t)(base + EXTNUM));       /* A=e000 0000 */
    a = (uint8_t)((a >> 1) | (a << 7));      /* rrc */
    a &= 0x80;
    t = (unsigned)a + c;
    c = (uint8_t)t;
    b = (uint8_t)(0 + b + (t > 0xFF));
    /* BC = 0000 000? errrr rrrr */
    a = rb((uint16_t)(base + EXTNUM));
    a = (uint8_t)((a >> 1) | (a << 7));
    a &= 0x0F;
    b = (uint8_t)(a + b);
    /* BC = 000? eeee errr rrrr */
    a = rb((uint16_t)(base + MODNUM));       /* A=XXX? mmmm */
    carry1 = 0;
    {
        int i;
        for (i = 0; i < 4; i++) {
            carry1 = (uint8_t)(a >> 7);
            a = (uint8_t)(a << 1);
        }
    }
    t = (unsigned)a + b;
    carry2 = t > 0xFF;
    b = (uint8_t)t;
    /* cy=?, BC = mmmm eeee errr rrrr */
    *bc_out = (uint16_t)((b << 8) | c);
    return (uint8_t)((carry1 | carry2) & 1);
}

static void getfilesize(void)
{
    /* compute logical file size for current fcb */
    uint16_t ranrec, hl, bc;

    search(EXTNUM);
    /* zero the receiving ranrec field */
    ranrec = (uint16_t)(info + RANREC);
    wb(ranrec, 0);
    wb((uint16_t)(ranrec + 1), 0);
    wb((uint16_t)(ranrec + 2), 0);
    while (!end_of_dir()) {
        int borrow;
        unsigned t;
        uint8_t e;
        /* current fcb addressed by dptr */
        hl = getdptra();         /* ready for compute size */
        e = compute_rr(hl, RECCNT, &bc);
        /* A=0000 000? BC = mmmm eeee errr rrrr; compare with memory, larger? */
        t = (unsigned)(bc & 0xFF) - rb(ranrec);
        borrow = t > 0xFF;
        t = (unsigned)(bc >> 8) - rb((uint16_t)(ranrec + 1)) - borrow;
        borrow = t > 0xFF;
        t = (unsigned)e - rb((uint16_t)(ranrec + 2)) - borrow;
        borrow = t > 0xFF;       /* carry if .fcb(ranrec) > directory */
        if (!borrow) {
            /* fcb is less or equal, fill from directory */
            wb((uint16_t)(ranrec + 2), e);
            wb((uint16_t)(ranrec + 1), (uint8_t)(bc >> 8));
            wb(ranrec, (uint8_t)bc);
        }
        searchn();
    }
}

static void setrandom(void)
{
    /* set random record from the current file control block */
    uint16_t bc;
    uint8_t a = compute_rr(info, NXTREC, &bc);
    uint16_t hl = (uint16_t)(info + RANREC);
    wb(hl, (uint8_t)bc);
    wb((uint16_t)(hl + 1), (uint8_t)(bc >> 8));
    wb((uint16_t)(hl + 2), a);
}

static void copy_dirloc(void)
{
    /* copy directory location to lret following delete, rename, ... ops */
    sta_ret(dirloc);
}

/* ------------------------------------------------------------------ */
/* individual function handlers                                        */

static void func0(void) { bios->wboot(); abort(); }

static void func1(void)
{
    /* return console character with echo */
    sta_ret(conech());
}

static void func2(uint8_t c) { tabout(c); }
static void func3(void) { sta_ret(bios->reader()); }
static void func4(uint8_t c) { bios->punch(c); }
static void func5(uint8_t c) { bios->list(c); }

static void func6(uint8_t c)
{
    /* direct console i/o - read if 0ffh */
    if ((uint8_t)(c + 1) == 0) {
        /* dirinp: status check; return 00 if not ready (retmon) */
        if (bios->consts() == 0)
            return;
        sta_ret(bios->conin());
        return;
    }
    if ((uint8_t)(c + 2) == 0) {
        bios->consts();          /* 0feH in C for status */
        return;
    }
    /* direct output function */
    bios->conout(c);
}

static void func7(void) { sta_ret(rb(IOLOC)); }       /* return io byte */
static void func8(uint8_t c) { wb(IOLOC, c); }        /* set i/o byte */
static void func9(void) { print(info); }              /* write line until $ */
static void func10(void) { read_line(); }
static void func11(void) { sta_ret(conbrk()); }       /* check console status */

static void func12(void) { sta_ret(DVERS); }          /* return version number */

static void func13(void)
{
    /* reset disk system - initialize to disk 0 */
    rodsk = 0;
    dlog = 0;
    curdsk = 0;                  /* note that usrcode remains unchanged */
    dmaad = TBUFF;
    setdata();                   /* to data dma address */
    select_();
}

static void func14(void) { curselect(); }             /* select disk info */

static void func15(void)
{
    /* open file */
    clrmodnum();                 /* clear the module number */
    reselect();
    open_();
}

static void func16(void)
{
    /* close file */
    reselect();
    close_();
}

static void func17(void)
{
    /* search for first occurrence of a file */
    uint8_t c = 0;               /* length assuming '?' true */
    if (rb(info) != '?') {
        /* normal search */
        if (rb(getexta()) != '?')
            clrmodnum();         /* module number zeroed */
        reselect();
        c = NAMLEN;
    }
    search(c);
    dir_to_user();               /* copy directory entry to user */
}

static void func18(void)
{
    /* search for next occurrence of a file name */
    info = searcha;
    reselect();
    searchn();
    dir_to_user();               /* copy directory entry to user */
}

static void func19(void)
{
    /* delete a file */
    reselect();
    delete_();
    copy_dirloc();
}

static void func20(void) { reselect(); seqdiskread(); }
static void func21(void) { reselect(); seqdiskwrite(); }

static void func22(void)
{
    /* make a file */
    clrmodnum();
    reselect();
    make_();
}

static void func23(void)
{
    /* rename a file */
    reselect();
    rename_();
    copy_dirloc();
}

static void func24(void) { sthl_ret(dlog); }          /* return the login vector */
static void func25(void) { sta_ret(curdsk); }         /* return selected disk number */

static void func26(void)
{
    /* set the subsequent dma address to info */
    dmaad = info;
    setdata();
}

static void func27(void) { sthl_ret(alloca_); }       /* return the login vector address */
static void func28(void) { set_ro(); }                /* write protect current disk */
static void func29(void) { sthl_ret(rodsk); }         /* return r/o bit vector */

static void func30(void)
{
    /* set file indicators */
    reselect();
    indicators();
    copy_dirloc();               /* lret=dirloc */
}

static void func31(void) { sthl_ret(dpbaddr); }       /* return address of disk parameter block */

static void func32(void)
{
    /* set user code */
    if (linfo == 0xFF) {
        /* interrogate user code instead */
        sta_ret(usrcode);
        return;
    }
    usrcode = linfo & 0x1F;
}

static void func33(void) { reselect(); randiskread(); }
static void func34(void) { reselect(); randiskwrite(); }
static void func35(void) { reselect(); getfilesize(); }
static void func36(void) { setrandom(); }

static void func37(void)
{
    /* reset drive: info = vector of drives to reset */
    uint16_t de = (uint16_t)(dlog & ~info);
    dlog = de;
    rodsk = (uint16_t)(rodsk & dlog);
}

static void func38(void) { }
static void func39(void) { }

static void func40(void)
{
    /* random disk write with zero fill of unallocated block */
    reselect();
    seqio = 2;
    if (rseek1(ON_FALSE) == 0)
        diskwrite();
}

/* arrive here at end of processing to return to user */
static void goback(void)
{
    if (resel != 0) {
        /* reselection may have taken place */
        wb(info, 0);
        if (fcbdsk != 0) {
            /* restore disk number */
            wb(info, fcbdsk);
            linfo = olddsk;
            curselect();
        }
    }
}

uint16_t bdos_call(uint8_t c, uint16_t de)
{
    uint8_t e = (uint8_t)de;

    if (bdos_trace && !(c == 2 || c == 6))
        fprintf(stderr, "bdos %02X %04X\n", c, de);
    info = de;
    linfo = e;                   /* linfo = low(info) - don't equ */
    aret = 0;                    /* return value defaults to 0000 */
    fcbdsk = 0;
    resel = 0;
    if (c < NFUNCS) {
        switch (c) {
        case 0:  func0(); break;
        case 1:  func1(); break;
        case 2:  func2(e); break;
        case 3:  func3(); break;
        case 4:  func4(e); break;
        case 5:  func5(e); break;
        case 6:  func6(e); break;
        case 7:  func7(); break;
        case 8:  func8(e); break;
        case 9:  func9(); break;
        case 10: func10(); break;
        case 11: func11(); break;
        case 12: func12(); break;
        case 13: func13(); break;
        case 14: func14(); break;
        case 15: func15(); break;
        case 16: func16(); break;
        case 17: func17(); break;
        case 18: func18(); break;
        case 19: func19(); break;
        case 20: func20(); break;
        case 21: func21(); break;
        case 22: func22(); break;
        case 23: func23(); break;
        case 24: func24(); break;
        case 25: func25(); break;
        case 26: func26(); break;
        case 27: func27(); break;
        case 28: func28(); break;
        case 29: func29(); break;
        case 30: func30(); break;
        case 31: func31(); break;
        case 32: func32(); break;
        case 33: func33(); break;
        case 34: func34(); break;
        case 35: func35(); break;
        case 36: func36(); break;
        case 37: func37(); break;
        case 38: func38(); break;
        case 39: func39(); break;
        case 40: func40(); break;
        }
        /* func6 direct input with nothing ready returns through retmon,
           which skips goback; resel is 0 there so it makes no difference */
        goback();
    } else {
        goback();
    }
    if (bdos_trace && !(c == 2 || c == 6))
        fprintf(stderr, "  = %04X\n", aret);
    return aret;
}
