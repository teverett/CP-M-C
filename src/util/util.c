#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t util_buffer[128];
static uint8_t util_fcbs[2][36];

static uint8_t upper(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
        return (uint8_t)(c - ('a' - 'A'));
    return c;
}

uint16_t util_pointer(const void *pointer)
{
    return (uint16_t)pointer;
}

uint16_t util_dma_buffer(void)
{
    return util_pointer(util_buffer);
}

uint16_t util_fcb_address(uint8_t index)
{
    return util_pointer(util_fcbs[index & 1]);
}

uint8_t util_call(uint8_t function, uint16_t address)
{
    return (uint8_t)bdos_call(function, address);
}

uint16_t util_call_pointer(uint8_t function, uint16_t address)
{
    return bdos_pointer_call(function, address);
}

uint8_t util_current_drive(void)
{
    return util_call(25, 0);
}

uint8_t util_args(uint8_t *args)
{
    uint8_t length = rb(UTIL_TAIL);
    uint8_t i;
    for (i = 0; i < length; i++)
        args[i] = rb((uint16_t)(UTIL_TAIL + 1 + i));
    args[length] = 0;
    return length;
}

void util_clear_fcb(uint16_t fcb)
{
    uint8_t i;
    for (i = 0; i < 36; i++)
        wb((uint16_t)(fcb + i), 0);
    for (i = 1; i < 12; i++)
        wb((uint16_t)(fcb + i), ' ');
}

uint8_t util_parse_fcb(const uint8_t *text, uint8_t length, uint8_t *pos,
                       uint16_t fcb, uint8_t default_drive,
                       uint8_t wildcards)
{
    uint8_t name = 0, ext = 0, drive = default_drive;
    uint8_t saw_name = 0;
    util_clear_fcb(fcb);
    if (*pos + 1 < length && text[*pos + 1] == ':') {
        uint8_t d = upper(text[*pos]);
        if (d < 'A' || d > 'P')
            return 0;
        drive = (uint8_t)(d - 'A' + 1);
        *pos = (uint8_t)(*pos + 2);
    }
    if (*pos >= length || text[*pos] == ' ' || text[*pos] == 0)
        return 0;
    wb(fcb, drive);
    while (*pos < length && text[*pos] != '.' && text[*pos] != ' ' &&
           text[*pos] != '=' && text[*pos] != ',') {
        uint8_t c = upper(text[(*pos)++]);
        saw_name = 1;
        if (c == '*' && wildcards) {
            while (name < 8)
                wb((uint16_t)(fcb + 1 + name++), '?');
        } else if (name < 8) {
            wb((uint16_t)(fcb + 1 + name++), c);
        }
    }
    if (*pos < length && text[*pos] == '.') {
        (*pos)++;
        while (*pos < length && text[*pos] != ' ' &&
               text[*pos] != '=' && text[*pos] != ',') {
            uint8_t c = upper(text[(*pos)++]);
            if (c == '*' && wildcards) {
                while (ext < 3)
                    wb((uint16_t)(fcb + 9 + ext++), '?');
            } else if (ext < 3) {
                wb((uint16_t)(fcb + 9 + ext++), c);
            }
        }
    }
    wb((uint16_t)(fcb + 32), 0);
    return saw_name;
}

void util_select(uint8_t drive)
{
    (void)util_call(14, drive);
}

void util_set_dma(uint16_t address)
{
    (void)util_call(26, address);
}

void util_print_char(uint8_t value)
{
    (void)util_call(2, value);
}

void util_print(const char *text)
{
    while (*text)
        util_print_char((uint8_t)*text++);
}

void util_print_newline(void)
{
    util_print_char(UTIL_CR);
    util_print_char(UTIL_LF);
}

void util_print_hex(uint8_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    util_print_char((uint8_t)digits[value >> 4]);
    util_print_char((uint8_t)digits[value & 15]);
}

void util_print_decimal(uint16_t value)
{
    static const uint16_t divisors[] = {10000, 1000, 100, 10, 1};
    uint8_t i, started = 0;
    for (i = 0; i < 5; i++) {
        uint8_t digit = (uint8_t)(value / divisors[i]);
        value = (uint16_t)(value % divisors[i]);
        if (digit || started || i == 4) {
            util_print_char((uint8_t)('0' + digit));
            started = 1;
        } else {
            util_print_char(' ');
        }
    }
}

void util_print_filename(uint16_t fcb)
{
    uint8_t i, c;
    for (i = 1; i < 12; i++) {
        c = (uint8_t)(rb((uint16_t)(fcb + i)) & 0x7F);
        if (i == 9 && rb((uint16_t)(fcb + 9)) != ' ')
            util_print_char('.');
        if (c != ' ')
            util_print_char(c);
    }
}
