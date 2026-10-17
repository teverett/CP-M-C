/*
 * plat_i386.c - platform services for the bare-metal i386 build, as run by
 * QEMU's PC machine:
 *   COM1 (0x3F8)  console          COM2 (0x2F8)  list/punch output
 *   COM3 (0x3E8)  diagnostics      IDE primary master (PIO): the disks
 *   0xF4          isa-debug-exit   (writing N makes QEMU exit with 2N+1)
 */
#include <stdint.h>
#include "cpm.h"
#include "plat.h"
#include "geom.h"
#include "machine.h"
#include "boot_hdr.h"

static inline void outb(uint16_t p, uint8_t v)
{
    __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(p));
}

static inline uint8_t inb(uint16_t p)
{
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(p));
    return v;
}

static inline void outw(uint16_t p, uint16_t v)
{
    __asm__ volatile("outw %0, %1" : : "a"(v), "Nd"(p));
}

static inline uint16_t inw(uint16_t p)
{
    uint16_t v;
    __asm__ volatile("inw %1, %0" : "=a"(v) : "Nd"(p));
    return v;
}

static inline void outl(uint16_t p, uint32_t v)
{
    __asm__ volatile("outl %0, %1" : : "a"(v), "Nd"(p));
}

/* ---- serial ports ------------------------------------------------- */

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8

static void uart_init(uint16_t base)
{
    outb(base + 1, 0x00);        /* no interrupts */
    outb(base + 3, 0x80);        /* DLAB on */
    outb(base + 0, 0x01);        /* 115200 baud */
    outb(base + 1, 0x00);
    outb(base + 3, 0x03);        /* 8N1 */
    outb(base + 2, 0xC7);        /* FIFO on */
    outb(base + 4, 0x03);
}

static void uart_put(uint16_t base, uint8_t c)
{
    while (!(inb(base + 5) & 0x20))
        ;
    outb(base, c);
}

static int uart_ready(uint16_t base)
{
    return inb(base + 5) & 1;
}

static uint8_t uart_get(uint16_t base)
{
    while (!uart_ready(base))
        ;
    return inb(base);
}

static void diag(const char *s)
{
    while (*s)
        uart_put(COM3, (uint8_t)*s++);
}

static void diag_hex(uint16_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    int i;
    for (i = 12; i >= 0; i -= 4)
        uart_put(COM3, (uint8_t)hex[(v >> i) & 15]);
}

/* ---- ATA PIO ------------------------------------------------------- */

#define ATA_DATA   0x1F0
#define ATA_COUNT  0x1F2
#define ATA_LBA0   0x1F3
#define ATA_LBA1   0x1F4
#define ATA_LBA2   0x1F5
#define ATA_DRIVE  0x1F6
#define ATA_CMD    0x1F7        /* status when read */

static int ata_wait(void)
{
    uint8_t s;
    int n;
    for (n = 0; n < 4000000; n++) {
        s = inb(ATA_CMD);
        if (s & 0x01)
            return -1;                     /* error */
        if (!(s & 0x80) && (s & 0x08))
            return 0;                      /* DRQ */
    }
    return -1;
}

static int ata_setup(uint32_t lba, uint8_t cmd)
{
    outb(ATA_DRIVE, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F)));
    outb(ATA_COUNT, 1);
    outb(ATA_LBA0, (uint8_t)lba);
    outb(ATA_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_CMD, cmd);
    return ata_wait();
}

static int ata_read(uint32_t lba, uint8_t *buf)
{
    int i;
    if (ata_setup(lba, 0x20))
        return -1;
    for (i = 0; i < 256; i++) {
        uint16_t w = inw(ATA_DATA);
        buf[2 * i] = (uint8_t)w;
        buf[2 * i + 1] = (uint8_t)(w >> 8);
    }
    return 0;
}

static int ata_write(uint32_t lba, const uint8_t *buf)
{
    int i;
    if (ata_setup(lba, 0x30))
        return -1;
    for (i = 0; i < 256; i++)
        outw(ATA_DATA, (uint16_t)(buf[2 * i] | (buf[2 * i + 1] << 8)));
    outb(ATA_CMD, 0xE7);                   /* flush cache */
    {
        int n;
        for (n = 0; n < 4000000 && (inb(ATA_CMD) & 0x80); n++)
            ;
    }
    return 0;
}

/* ---- run parameters and the console script ----------------------------- */

static struct boot_hdr hdr;
static int interactive;
static uint32_t script_pos;
static uint8_t secbuf[512];
static uint32_t script_cached = 0xFFFFFFFFu;
static int32_t inject_reads;

void plat_exit(int code)
{
    outl(0xF4, (uint32_t)code);
    for (;;)
        __asm__ volatile("hlt");
}

void plat_halt(uint16_t pc)
{
    diag("cpm: halted at ");
    diag_hex(pc);
    diag("\n");
    plat_exit(4);
}

void plat_limit(void)
{
    diag("cpm: instruction limit reached\n");
    plat_exit(3);
}

static uint8_t script_byte(uint32_t pos)
{
    uint32_t lba = SCRIPT_LBA + pos / 512;
    if (lba != script_cached) {
        if (ata_read(lba, secbuf))
            plat_exit(5);
        script_cached = lba;
    }
    return secbuf[pos % 512];
}

void plat_putc(uint8_t c)
{
    uart_put(COM1, c);
}

uint8_t plat_getc(void)
{
    if (interactive)
        return uart_get(COM1);
    if (script_pos >= hdr.script_len)
        plat_exit(0);                      /* end of script */
    return script_byte(script_pos++);
}

uint8_t plat_kbhit(void)
{
    if (interactive)
        return (uint8_t)uart_ready(COM1);
    return (hdr.flags & BF_TYPEAHEAD) && script_pos < hdr.script_len;
}

void plat_list(uint8_t c)   { uart_put(COM2, c); }
void plat_punch(uint8_t c)  { (void)c; }
uint8_t plat_reader(void)   { return 0x1A; }

int plat_disk_present(uint8_t d)
{
    return d < NGEOMS && ((hdr.present >> d) & 1);
}

uint8_t plat_disk_rw(int write, uint8_t d, uint16_t trk, uint16_t sec,
                     uint16_t dma)
{
    const geom *g = &geoms[d];
    uint32_t off;
    uint32_t lba;
    int i;

    if (!write && hdr.inject_count >= 0 && ++inject_reads == hdr.inject_count &&
        hdr.inject_drive < NGEOMS) {
        uint32_t o = hdr.inject_off;
        uint32_t l = DRIVE_LBA(hdr.inject_drive) + o / 512;
        if (!ata_read(l, secbuf)) {
            secbuf[o % 512] = (uint8_t)hdr.inject_val;
            ata_write(l, secbuf);
        }
        script_cached = 0xFFFFFFFFu;
    }
    if (trk >= g->tracks || sec < 1 || sec > g->spt ||
        (int32_t)trk == hdr.err_track)
        return 1;
    off = ((uint32_t)trk * g->spt + (uint32_t)(sec - 1)) * 128u;
    lba = DRIVE_LBA(d) + off / 512;
    if (ata_read(lba, secbuf))
        return 1;
    script_cached = 0xFFFFFFFFu;
    off %= 512;
    if (write) {
        for (i = 0; i < 128; i++)
            secbuf[off + i] = rb((uint16_t)(dma + i));
        if (ata_write(lba, secbuf))
            return 1;
    } else {
        for (i = 0; i < 128; i++)
            wb((uint16_t)(dma + i), secbuf[off + i]);
    }
    return 0;
}

/* ---- entry --------------------------------------------------------------- */

void kmain(void)
{
    uart_init(COM1);
    uart_init(COM2);
    uart_init(COM3);
    if (ata_read(0, secbuf)) {
        diag("cpm: no system disk\n");
        plat_exit(5);
    }
    {
        int i;
        for (i = 0; i < (int)sizeof hdr; i++)
            ((uint8_t *)&hdr)[i] = secbuf[i];
    }
    if (hdr.magic != BOOT_MAGIC) {
        diag("cpm: bad system disk header\n");
        plat_exit(5);
    }
    interactive = hdr.script_len == NO_SCRIPT;
    script_cached = 0xFFFFFFFFu;
    machine_boot();
    machine_run();
}
