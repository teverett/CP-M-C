/*
 * ccp.c - CP/M 2.2 console command processor, translated routine by
 * routine from ccp.asm.
 *
 * Data that the BDOS or a transient program can see (the command buffer,
 * the submit and command FCBs) lives in mem[] at the same offsets the
 * original used; everything else is a C variable.  "jmp ccp" from deep
 * inside a command is a longjmp back to the prompt loop.
 *
 * Original: Copyright (c) 1976-1980 Digital Research.
 */
#include <stdint.h>
#include "cpm.h"

#define CR     13
#define LF     10
#define LA     0x5F             /* left arrow */
#define EOFILE 0x1A

#define RCHARF  1
#define PCHARF  2
#define PBUFF   9
#define RBUFF   10
#define BREAKF  11
#define INITF   13
#define SELF    14
#define OPENF   15
#define CLOSEF  16
#define SEARF   17
#define SEARNF  18
#define DELF    19
#define DREADF  20
#define DWRITF  21
#define MAKEF   22
#define RENF    23
#define CSELF   25
#define DMAF    26
#define USERF   32

#define SYSFILE 10

#define BUFF    0x0080
#define FCB     0x005C
#define TRAN_   0x0100
#define TRANM   CCP_BASE        /* load limit: the CCP itself */

/* ccp image layout (see the header comment of ccp.asm) */
#define MAXLEN  CCP_MAXLEN
#define COMLEN  CCP_COMLEN
#define COMBUF  CCP_COMBUF

/* data areas.  The original keeps these after the CCP's own stack. */
#define SUBMIT  (CCP_BASE + 0x7AB)
#define SUBFCB  (SUBMIT + 1)
#define SUBMOD  (SUBFCB + 14)
#define SUBRC   (SUBFCB + 15)
#define SUBCR   (SUBFCB + 32)
#define COMFCB  (CCP_BASE + 0x7CD)
#define COMREC  (COMFCB + 32)

static uint16_t comaddr;        /* address of next to char to scan */
static uint16_t staddr;         /* starting address of current fillfcb request */
static uint8_t  dcnt;           /* disk directory count (used for error codes) */
static uint8_t  cdisk;          /* current disk */
static uint8_t  sdisk;          /* selected disk for current operation */
static uint8_t  bptr;           /* buffer pointer */

static cpm_jmpbuf ccp_jmp;

/* jmp ccp */
static void ccp_restart(void) NORETURN;
static void ccp_restart(void)
{
    cpm_longjmp(ccp_jmp);
}

/* ------------------------------------------------------------------ */
/* utility procedures                                                  */

static uint8_t bdos(uint8_t c, uint16_t de)
{
    return (uint8_t)bdos_call(c, de);
}

static void printchar(uint8_t a)
{
    bdos(PCHARF, a);
}

#define printbc printchar       /* print character, but save b,c registers */

static void crlf(void)
{
    printbc(CR);
    printbc(LF);
}

static void blank(void)
{
    printbc(' ');
}

static void prin0(uint16_t hl)
{
    /* print string starting at hl until next 00 entry */
    for (;;) {
        uint8_t a = rb(hl);
        if (a == 0)
            return;
        hl++;
        printchar(a);
    }
}

static void print(const char *s)
{
    /* print string until next 00 entry, after a new line */
    crlf();
    while (*s)
        printchar((uint8_t)*s++);
}

static uint8_t initialize(void)
{
    return bdos(INITF, 0);
}

static void select_(uint8_t a)
{
    bdos(SELF, a);
}

/* Calls that do inr a on return: result 0 means "not found/failed" */
static uint8_t bdos_inr(uint8_t c, uint16_t de)
{
    uint8_t a = bdos(c, de);
    dcnt = a;
    return (uint8_t)(a + 1);
}

static uint8_t open_(uint16_t de)
{
    /* open the file given by d,e */
    return bdos_inr(OPENF, de);
}

static uint8_t openc(void)
{
    /* open comfcb */
    wb(COMREC, 0);               /* clear next record to read */
    return open_(COMFCB);
}

static uint8_t close_(uint16_t de)
{
    return bdos_inr(CLOSEF, de);
}

static uint8_t search(uint16_t de)
{
    return bdos_inr(SEARF, de);
}

static uint8_t searchn(void)
{
    /* search for the next occurrence of the file given by d,e */
    return bdos_inr(SEARNF, 0);
}

static uint8_t searchcom(void)
{
    /* search for comfcb file */
    return search(COMFCB);
}

static uint8_t delete_(uint16_t de)
{
    return bdos(DELF, de);
}

/* bdos_cond: result in A, nonzero = failure/eof */
static uint8_t diskread(uint16_t de)
{
    return bdos(DREADF, de);
}

static uint8_t diskreadc(void)
{
    return diskread(COMFCB);
}

static uint8_t diskwrite(uint16_t de)
{
    return bdos(DWRITF, de);
}

static uint8_t make_(uint16_t de)
{
    return bdos_inr(MAKEF, de);
}

static void renam(uint16_t de)
{
    bdos(RENF, de);
}

static uint8_t getuser(void)
{
    /* return current user code in a */
    return bdos(USERF, 0xFF);
}

static void setuser(uint8_t e)
{
    bdos(USERF, e);
}

static void saveuser(void)
{
    /* save user#/disk# before possible ^c or transient */
    uint8_t a = getuser();
    a = (uint8_t)(a << 4);
    a |= cdisk;                  /* 4b=user, 4b=disk */
    wb(DISKA, a);                /* stored away in memory for later */
}

static void setdiska(void)
{
    wb(DISKA, cdisk);
}

static uint8_t translate(uint8_t a)
{
    /* translate character in register A to upper case */
    if (a < 0x61)
        return a;
    if (a >= 0x7B)
        return a;
    return (uint8_t)(a & 0x5F);
}

static uint8_t break_key(void)
{
    /* check for a character ready at the console */
    uint8_t a = bdos(BREAKF, 0);
    if (a == 0)
        return 0;
    return bdos(RCHARF, 0);      /* character cleared */
}

static uint8_t cselect(void)
{
    /* get the currently selected drive number */
    return bdos(CSELF, 0);
}

static void setdma(uint16_t de)
{
    bdos(DMAF, de);
}

static void setdmabuff(void)
{
    /* set default buffer dma address */
    setdma(BUFF);
}

static void del_sub(void)
{
    /* delete the submit file, and set submit flag to false */
    if (rb(SUBMIT) == 0)
        return;
    wb(SUBMIT, 0);               /* submit flag is set to false */
    select_(0);                  /* on drive a to erase file */
    delete_(SUBFCB);
    select_(cdisk);              /* back to original drive */
}

static void readcom(void)
{
    /* read the next command into the command buffer;
       check for submit file */
    uint16_t hl;
    uint8_t b, a;

    if (rb(SUBMIT) != 0) {
        /* scanning a submit file; change drives to open and read the file */
        if (cdisk != 0)
            select_(0);
        /* have to open again in case xsub present */
        if (open_(SUBFCB) == 0)
            goto nosub;          /* skip if no sub */
        a = (uint8_t)(rb(SUBRC) - 1);       /* read last record(s) first */
        wb(SUBCR, a);            /* current record to read */
        if (diskread(SUBFCB) != 0)
            goto nosub;          /* end of file if last record */
        /* disk read is ok, transfer to combuf */
        for (b = 0; b < 128; b++)
            wb((uint16_t)(COMLEN + b), rb((uint16_t)(BUFF + b)));
        /* line is transferred, close the file with a deleted record */
        wb(SUBMOD, 0);           /* clear fwflag */
        wb(SUBRC, (uint8_t)(rb(SUBRC) - 1));   /* one less record */
        if (close_(SUBFCB) == 0)
            goto nosub;
        /* close went ok, return to original drive */
        if (cdisk != 0)
            select_(cdisk);
        /* print to the 00 */
        prin0(COMBUF);
        if (break_key() == 0)
            goto noread;
        del_sub();               /* break key depressed */
        ccp_restart();
    }

nosub:
    /* no submit file */
    del_sub();
    /* translate to upper case, store zero at end */
    saveuser();                  /* user # save in case control c */
    bdos(RBUFF, MAXLEN);
    setdiska();                  /* no control c, so restore diska */

noread:
    /* enter here from submit file; set the last character to zero for
       later scans */
    hl = COMLEN;
    b = rb(hl);                  /* length is in b */
    for (;;) {
        hl++;
        if (b == 0)
            break;               /* end of scan? */
        wb(hl, translate(rb(hl)));
        b--;
    }
    /* end of scan, h,l address end of command */
    wb(hl, 0);                   /* store a zero */
    comaddr = COMBUF;            /* ready to scan to zero */
}

/* ------------------------------------------------------------------ */
/* serialization                                                       */

static const uint8_t serial[6] = { 0, 0, 0, 0, 0, 0 };

static void badserial(void) NORETURN;
static void badserial(void)
{
    /* the original stored DI/HLT at the CCP base and jumped to it */
    cpm_halt();
}

static void serialize(void)
{
    /* check serialization (six bytes at the BDOS base) */
    int i;
    for (i = 0; i < 6; i++)
        if (serial[i] != rb((uint16_t)(CCP_SERIAL + i)))
            badserial();
}

static void comerr(void) NORETURN;
static void comerr(void)
{
    /* error in command string starting at position 'staddr' and ending
       with first delimiter */
    uint16_t hl;
    uint8_t a;
    crlf();                      /* space to next line */
    hl = staddr;                 /* h,l address first to print */
    for (;;) {                   /* print characters until blank or zero */
        a = rb(hl);
        if (a == ' ')
            break;
        if (a == 0)
            break;
        printchar(a);
        hl++;
    }
    /* print question mark, and delete sub file */
    printchar('?');
    crlf();
    del_sub();
    ccp_restart();               /* restart with next command */
}

/* ------------------------------------------------------------------ */
/* fcb scan and fill subroutine                                        */

/* delim: look for a delimiter.  Returns true (zero flag) if the character
   at de is a delimiter; *a gets the character. */
static int delim(uint16_t de, uint8_t *a)
{
    *a = rb(de);                 /* not the last element */
    if (*a == 0)
        return 1;
    if (*a < ' ')                /* non graphic */
        comerr();
    if (*a == ' ')               /* treat blank as delimiter */
        return 1;
    if (*a == '=' || *a == LA || *a == '.' || *a == ':' || *a == ';' ||
        *a == '<' || *a == '>')
        return 1;
    return 0;                    /* delimiter not found */
}

static uint16_t deblank(uint16_t de)
{
    /* deblank the input line (end of line treated as blank) */
    while (rb(de) == ' ')
        de++;
    return de;
}

static uint8_t fillfcb(uint8_t offset)
{
    /* fill the comfcb, indexed by A (0 or 16); returns number of ?'s */
    uint16_t hl = (uint16_t)(COMFCB + offset);
    uint16_t de;
    uint8_t a, b;

    sdisk = 0;                   /* clear selected disk (in case A:...) */
    de = deblank(comaddr);       /* to first non-blank character */
    staddr = de;                 /* in case of errors */
    /* look for preceding file name A: B: ... */
    a = rb(de);                  /* use current disk if empty command */
    if (a == 0)
        goto setcur0;
    b = (uint8_t)(a - ('A' - 1));    /* disk name held in b if : follows */
    de++;                        /* set disk name if : */
    if (rb(de) == ':')
        goto setdsk;
                                 /* setcur: set current disk */
    de--;                        /* back to first character of command */
setcur0:
    wb(hl, cdisk);
    goto setname;
setdsk:                          /* set disk to name in register b */
    sdisk = b;                   /* mark as disk selected */
    wb(hl, b);                   /* past the : */
    de++;
setname:                         /* set the file name field */
    b = 8;                       /* file name length (max) */
setnam0:
    if (delim(de, &a))
        goto padname;
    hl++;                        /* must be ?'s */
    if (a != '*')
        goto setnam1;
    wb(hl, '?');                 /* to dec count */
    goto setnam2;
setnam1:
    wb(hl, a);                   /* store character to fcb */
    de++;
setnam2:
    b--;                         /* count down length */
    if (b != 0)
        goto setnam0;
    /* end of name, truncate remainder */
trname:
    if (delim(de, &a))           /* set type field if delimiter */
        goto setty;
    de++;
    goto trname;
padname:
    hl++;
    wb(hl, ' ');
    b--;
    if (b != 0)
        goto padname;
setty:                           /* set the type field */
    b = 3;                       /* skip the type field if no . */
    if (a != '.')
        goto padty;
    de++;                        /* past the ., to the file type field */
setty0:                          /* set the field from the command buffer */
    if (delim(de, &a))
        goto padty;
    hl++;
    if (a != '*')
        goto setty1;
    wb(hl, '?');                 /* since * specified */
    goto setty2;
setty1:                          /* not a *, so copy to type field */
    wb(hl, a);
    de++;
setty2:                          /* decrement count and go again */
    b--;
    if (b != 0)
        goto setty0;
    /* end of type field, truncate */
trtyp:
    if (delim(de, &a))
        goto efill;
    de++;
    goto trtyp;
padty:                           /* pad the type field with blanks */
    hl++;
    wb(hl, ' ');
    b--;
    if (b != 0)
        goto padty;
efill:                           /* end of the filename/filetype fill */
    /* fill the remaining fields for the fcb */
    b = 3;
    do {
        hl++;
        wb(hl, 0);
        b--;
    } while (b != 0);
    comaddr = de;                /* set new starting point */

    /* recover the start address of the fcb and count ?'s */
    hl = (uint16_t)(COMFCB + offset);
    b = 0;
    {
        uint8_t c = 11;
        do {
            hl++;
            if (rb(hl) == '?')
                b++;             /* ? found, count it in b */
            c--;
        } while (c != 0);
    }
    return b;
}

static uint8_t fillfcb0(void)
{
    /* equivalent to fillfcb(0) */
    return fillfcb(0);
}

/* intrinsic function names (all are four characters) */
static const char intvec[] = "DIR ERA TYPESAVEREN USER";
#define INTLEN 6

static uint8_t intrinsic(void)
{
    /* look for intrinsic functions (comfcb has been filled) */
    uint8_t c;
    for (c = 0; c < INTLEN; c++) {
        uint8_t i, b = 4;
        const char *hl = &intvec[c * 4];
        uint16_t de = (uint16_t)(COMFCB + 1);   /* beginning of name */
        for (i = 0; i < b; i++)
            if (rb((uint16_t)(de + i)) != (uint8_t)hl[i])
                break;
        if (i == b) {
            /* complete match on name, check for blank in fcb */
            if (rb((uint16_t)(de + 4)) == ' ')
                return c;        /* with intrinsic number in a */
        }
    }
    return c;
}

/* utility subroutines for intrinsic handlers */

static void readerr(void) { print("READ ERROR"); }
static void nofile(void)  { print("NO FILE"); }

static uint8_t getnumber(void)
{
    /* read a number from the command line */
    uint16_t hl;
    uint8_t a, b, c, d;

    fillfcb0();                  /* should be number */
    if (sdisk != 0)              /* cannot be prefixed */
        comerr();
    /* convert the byte value in comfcb to binary */
    hl = COMFCB + 1;
    c = 11;
    b = 0;                       /* value accumulated in b */
    for (;;) {
        a = rb(hl);
        if (a == ' ')
            break;
        /* more to scan, convert char to binary and add */
        hl++;                    /* valid? */
        a = (uint8_t)(a - '0');
        if (a >= 10)
            comerr();
        d = a;                   /* save value */
        if (b & 0xE0)            /* mult by 10 */
            comerr();
        {
            unsigned t = (unsigned)((uint8_t)(b << 3)) + b;     /* *8 + b */
            if (t > 0xFF)
                comerr();
            t += b;                                              /* *8+*2 = *10 */
            if (t > 0xFF)
                comerr();
            t += d;                                              /* +digit */
            if (t > 0xFF)
                comerr();
            b = (uint8_t)t;      /* for another digit */
        }
        c--;
        if (c == 0)
            return b;
    }
    /* end of digits, check for all blanks */
    do {
        if (rb(hl) != ' ')       /* blanks? */
            comerr();
        hl++;
        c--;
    } while (c != 0);
    return b;                    /* recover value */
}

static void move0(uint16_t hl, uint16_t de, uint8_t b)
{
    /* move b characters from h,l to d,e addresses */
    do {
        wb(de, rb(hl));
        hl++;
        de++;
        b--;
    } while (b != 0);
}

static uint8_t addhcf(uint8_t a, uint8_t c)
{
    /* buff + a + c to h,l followed by fetch */
    return rb((uint16_t)(BUFF + (uint8_t)(a + c)));
}

static void setdisk(void)
{
    /* change disks for this command, if requested */
    uint8_t a;
    wb(COMFCB, 0);               /* clear disk name from fcb */
    a = sdisk;                   /* no action if not specified */
    if (a == 0)
        return;
    a--;                         /* already selected */
    if (a == cdisk)
        return;
    select_(a);
}

static void resetdisk(void)
{
    /* return to original disk after command */
    uint8_t a = sdisk;           /* no action if not selected */
    if (a == 0)
        return;
    a--;                         /* same disk */
    if (a == cdisk)
        return;
    select_(cdisk);
}

static void endcom(void)
{
    /* end of intrinsic command: check for garbage at end of line */
    uint8_t a;
    fillfcb0();
    a = (uint8_t)(rb(COMFCB + 1) - ' ');
    a |= sdisk;
    /* 0 in accumulator if no disk selected, and blank fcb */
    if (a != 0)
        comerr();
}

static void retcom(void)
{
    /* reset disk before end of command check */
    resetdisk();
    endcom();
}

/* ------------------------------------------------------------------ */
/* individual intrinsics                                               */

static void direct(void)
{
    /* directory search */
    uint8_t e, b, c, a, m;
    uint16_t hl;

    fillfcb0();                  /* comfcb gets file name */
    setdisk();                   /* change disk drives if requested */
    hl = COMFCB + 1;             /* may be empty request */
    if (rb(hl) == ' ') {         /* skip fill of ??? if not blank */
        /* set comfcb to all ??? for current disk */
        for (b = 11; b != 0; b--) {      /* length of fill ????????.??? */
            wb(hl, '?');
            hl++;
        }
    }
    /* not a blank request, must be in comfcb */
    e = 0;                       /* E counts directory entries */
    if (searchcom() == 0) {      /* first one has been found */
        nofile();                /* not found message */
        goto endir;
    }
    for (;;) {
        /* dir2: found, but may be system file */
        a = dcnt;                /* get the location of the element */
        a = (uint8_t)((a >> 3) | (a << 5));     /* rrc x3 */
        a &= 0x60;
        c = a;                   /* base index into buff for dir entry */
        a = addhcf(SYSFILE, c);
        if (a & 0x80)            /* skip if system file */
            goto dir6;
        /* c holds index into buffer; another fcb found, new line? */
        a = e;
        e++;
        /* e=0,1,2,3,...new line if mod 4 = 0 */
        m = a & 3;               /* and save the test */
        if (m == 0) {
            crlf();
            a = cselect();       /* current disk in A */
            printbc((uint8_t)(a + 'A'));
            printbc(':');
        } else {
            blank();             /* after last one */
            printbc(':');
        }
        blank();
        /* compute position of name in buffer */
        b = 1;                   /* start with first character of name */
        for (;;) {
            a = addhcf(b, c) & 0x7F;     /* mask flags */
            /* may delete trailing blanks */
            if (a == ' ') {      /* check for blank type */
                if (m == 3) {    /* may be 3rd item; place blank at end if not */
                    a = addhcf(9, c) & 0x7F;    /* first char of type */
                    if (a == ' ')
                        goto dir5;
                }
                a = ' ';         /* restore trailing filename chr */
            }
            printbc(a);          /* char printed */
            b++;
            if (b >= 12)
                goto dir5;
            /* check for break between names */
            if (b == 9)
                blank();         /* print a blank between names */
        }
dir5:   ;                        /* end of current entry */
dir6:
        if (break_key() != 0)    /* check for interrupt at keyboard */
            goto endir;          /* abort directory search */
        if (searchn() == 0)      /* for another entry */
            goto endir;
    }
endir:                           /* end of directory scan */
    retcom();
}

static const char ermsg[] = "ALL (Y/N)?";

static void erase(void)
{
    uint8_t a;
    a = fillfcb0();              /* cannot be all ???'s */
    if (a == 11) {
        /* erasing all of the disk */
        print(ermsg);
        readcom();
        wb(COMLEN, (uint8_t)(rb(COMLEN) - 1));  /* bad input */
        if (rb(COMLEN) != 0)
            return;              /* jnz ccp */
        if (rb(COMLEN + 1) != 'Y')
            return;
        /* ok, erase the entire diskette */
        comaddr = COMLEN + 2;    /* otherwise error at retcom */
    }
    /* erasefile: */
    setdisk();
    a = delete_(COMFCB);
    a++;                         /* 255 returned if not found */
    if (a == 0)
        nofile();                /* no file message if so */
    retcom();
}

static void type(void)
{
    uint8_t a;
    if (fillfcb0() != 0)         /* don't allow ?'s in file name */
        comerr();
    setdisk();                   /* open the file */
    if (openc() == 0)            /* zero flag indicates not found */
        goto typerr;
    /* file opened, read 'til eof */
    crlf();                      /* read first buffer */
    bptr = 255;
    for (;;) {
        /* type0: loop on bptr */
        if (bptr >= 128) {       /* end buffer */
            /* read another buffer full */
            a = diskreadc();
            if (a != 0)          /* hard end of file */
                goto type_eof;
            bptr = 0;
        }
        /* type1: read character at bptr and print */
        a = rb((uint16_t)(BUFF + bptr));
        bptr++;                  /* bptr = bptr + 1 */
        if (a == EOFILE)
            goto done;
        printchar(a);
        if (break_key() != 0)    /* abort if break */
            goto done;
    }
type_eof:                          /* end of file, check for errors */
    a--;
    if (a == 0)
        goto done;
    readerr();
typerr:
    resetdisk();
    comerr();
done:
    retcom();
}

static const char fullmsg[] = "NO SPACE";

static void save(void)
{
    uint8_t pages, a;
    uint16_t hl, de;

    pages = getnumber();         /* value to register a */
    /* should be followed by a file to save the memory image */
    if (fillfcb0() != 0)         /* cannot be ambiguous */
        comerr();
    setdisk();                   /* may be a disk change */
    delete_(COMFCB);             /* existing file removed */
    if (make_(COMFCB) == 0)      /* create a new file on disk */
        goto saverr;             /* no directory space */
    wb(COMREC, 0);               /* clear next record field */
    /* #pages to write is in a, change to #sectors */
    hl = (uint16_t)(pages * 2);
    de = TRAN_;                  /* h,l is sector count, d,e is load address */
    for (;;) {
        if (hl == 0)             /* may be completed */
            break;
        hl--;                    /* sector count = sector count - 1 */
        setdma(de);              /* current dma address set */
        a = diskwrite(COMFCB);
        de = (uint16_t)(de + 128);
        if (a != 0)              /* may be disk full case */
            goto saverr;
    }
    /* end of dump, close the file */
    a = close_(COMFCB);
    a++;                         /* 255 becomes 00 if error */
    if (a != 0)
        goto retsave;            /* for another command */
saverr:                          /* must be full or read only disk */
    print(fullmsg);
retsave:
    /* reset dma buffer */
    setdmabuff();
    retcom();
}

static const char renmsg[] = "FILE EXISTS";

static void rename_(void)
{
    /* rename a file on a specific disk */
    uint8_t b;
    uint16_t de;
    uint8_t saved_sdisk;
    uint8_t a;

    if (fillfcb0() != 0)         /* must be unambiguous */
        comerr();
    saved_sdisk = sdisk;         /* save for later compare */
    setdisk();                   /* disk selected */
    if (searchcom() != 0)        /* is new name already there? */
        goto renerr3;
    /* file doesn't exist, move to second half of fcb */
    move0(COMFCB, COMFCB + 16, 16);
    /* check for = or left arrow */
    de = deblank(comaddr);
    a = rb(de);
    if (a != '=' && a != LA)
        goto renerr2;
    /* ren1: past delimiter */
    comaddr = (uint16_t)(de + 1);
    /* proper delimiter found */
    if (fillfcb0() != 0)
        goto renerr2;
    /* check for drive conflict */
    b = saved_sdisk;             /* previous drive number */
    if (sdisk != 0) {
        /* drive name was specified.  same one? */
        a = sdisk;
        sdisk = b;
        if (a != b)
            goto renerr2;
    }
    sdisk = b;                   /* store the name in case drives switched */
    wb(COMFCB, 0);               /* is old file there? */
    if (searchcom() == 0)
        goto renerr1;
    /* everything is ok, rename the file */
    renam(COMFCB);
    retcom();
    return;
renerr1:                         /* no file on disk */
    nofile();
    retcom();
    return;
renerr2:                         /* ambiguous reference/name conflict */
    resetdisk();
    comerr();
renerr3:                         /* file already exists */
    print(renmsg);
    retcom();
}

static void user(void)
{
    /* set user number */
    uint8_t a = getnumber();     /* leaves the value in the accumulator */
    if (a >= 16)                 /* must be between 0 and 15 */
        comerr();
    if (rb(COMFCB + 1) == ' ')
        comerr();
    setuser(a);                  /* new user number set */
    endcom();
}

static const char loadmsg[] = "BAD LOAD";
static const char comtype[] = "COM";

static void userfunc(void)
{
    uint8_t a, b;
    uint16_t hl, de;

    serialize();                 /* check serialization */
    /* load user function and set up for execution */
    if (rb(COMFCB + 1) == ' ') {
        /* no file name, but may be disk switch */
        a = sdisk;               /* no disk name if 0 */
        if (a == 0) {
            endcom();
            return;
        }
        a--;                     /* set user/disk */
        cdisk = a;
        setdiska();
        select_(a);
        endcom();
        return;
    }
    /* user0: file name is present */
    de = COMFCB + 9;             /* type ' ' */
    if (rb(de) != ' ')
        comerr();
    setdisk();
    {
        int i;
        for (i = 0; i < 3; i++)
            wb((uint16_t)(de + i), (uint8_t)comtype[i]);   /* file type is set to .com */
    }
    if (openc() == 0)
        goto userer;
    /* file opened properly, read it into memory */
    hl = TRAN_;                  /* transient program base */
    for (;;) {
        setdma(hl);              /* load0: hl is the dma address */
        a = diskread(COMFCB);
        if (a != 0)
            goto load1;
        /* sector loaded, set new dma address and compare */
        hl = (uint16_t)(hl + 128);
        if (hl >= TRANM)         /* has the load overflowed? */
            goto loaderr;
    }
load1:                           /* end file is 1 */
    a--;
    if (a != 0)
        goto loaderr;
    resetdisk();                 /* back to original disk */
    fillfcb0();
    wb(COMFCB, sdisk);           /* drive number set */
    fillfcb(16);                 /* move entire fcb to memory */
    wb(COMFCB + 16, sdisk);
    wb(COMREC, 0);               /* record number set to zero */
    move0(COMFCB, FCB, 33);
    /* move command line to buff */
    hl = COMBUF;
    for (;;) {
        a = rb(hl);
        if (a == 0 || a == ' ')
            break;
        hl++;                    /* for another scan */
    }
    /* first blank position found */
    b = 0;                       /* ready for the move */
    de = BUFF + 1;
    for (;;) {
        a = rb(hl);
        wb(de, a);
        if (a == 0)
            break;
        /* more to move */
        b++;
        hl++;
        de++;
    }
    /* b has character count */
    wb(BUFF, b);
    crlf();
    /* now go to the loaded program */
    setdmabuff();                /* default dma */
    saveuser();                  /* user code saved; diska contains user code */
    ccp_run_tpa();               /* gone to the loaded program */
    /* may come back here */
    setdiska();
    select_(cdisk);
    return;

userer:                          /* arrive here on command error */
    resetdisk();
    comerr();
loaderr:                         /* cannot load the program */
    print(loadmsg);
    retcom();
}

/* ------------------------------------------------------------------ */
/* ccp                                                                 */

void ccp_load(void)
{
    /* The CCP image as loaded from disk: command buffer and submit FCB */
    static const char copyright[] = "COPYRIGHT (C) 1979, DIGITAL RESEARCH  ";
    int i;
    wb(MAXLEN, 127);             /* max buffer length */
    wb(COMLEN, 0);               /* command length */
    for (i = 0; i < 128; i++)
        wb((uint16_t)(COMBUF + i), 0);
    for (i = 0; i < 16; i++)
        wb((uint16_t)(COMBUF + i), ' ');
    for (i = 0; copyright[i]; i++)
        wb((uint16_t)(COMBUF + 16 + i), (uint8_t)copyright[i]);
    /* submit file control block */
    wb(SUBMIT, 0);
    for (i = 0; i < 34; i++)
        wb((uint16_t)(SUBFCB + i), 0);
    wb(SUBFCB, 0);
    {
        static const char nm[] = "$$$     SUB";
        for (i = 0; i < 11; i++)
            wb((uint16_t)(SUBFCB + 1 + i), (uint8_t)nm[i]);
    }
    for (i = 0; i < 36; i++)
        wb((uint16_t)(COMFCB + i), 0);
    comaddr = COMBUF;
    staddr = 0;
    dcnt = cdisk = sdisk = bptr = 0;
}

void ccp_start(uint8_t c)
{
    /* enter here from boot loader; c = (user code << 4) | disk# */
    volatile int have_cmd;
    uint8_t a;

    setuser((uint8_t)(c >> 4));  /* user code selected */
    /* initialize for this user, get $ flag */
    a = initialize();            /* 0ffh in accum if $ file present */
    wb(SUBMIT, a);               /* submit flag set if $ file present */
    cdisk = c & 0x0F;            /* clears user code nibble */
    select_(cdisk);              /* proper disk is selected, now check sub files */
    /* check for initial command */
    have_cmd = rb(COMLEN) != 0;  /* assume typed already */

    cpm_setjmp(ccp_jmp);         /* "ccp:" - enter on each command or error */
    for (;;) {
        uint8_t i;
        if (!have_cmd) {
            crlf();              /* print d> prompt, where d is disk name */
            a = cselect();       /* get current disk number */
            printchar((uint8_t)(a + 'A'));
            printchar('>');
            readcom();           /* command buffer filled */
        }
        have_cmd = 0;
        /* ccp0: (enter here from initialization with command full) */
        setdma(BUFF);            /* default dma address at buff */
        cdisk = cselect();       /* current disk number saved */
        if (fillfcb0() != 0)     /* command fcb filled */
            comerr();            /* the name cannot be an ambiguous reference */
        if (sdisk != 0) {
            userfunc();
            continue;
        }
        /* check for an intrinsic function */
        i = intrinsic();
        switch (i) {
        case 0: direct(); break;
        case 1: erase(); break;
        case 2: type(); break;
        case 3: save(); break;
        case 4: rename_(); break;
        case 5: user(); break;
        default: userfunc(); break;
        }
    }
}
