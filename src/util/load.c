#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t args[129];
static uint16_t source_fcb, dest_fcb, dma;
static uint16_t source_index;
static uint8_t source_read_error;

static int load_char(void)
{
    if (source_index >= 128) {
        util_set_dma(dma);
        {
            uint8_t result = util_call(20, source_fcb);
            if (result != 0) {
                source_read_error = (uint8_t)(result != 1);
                return -1;
            }
        }
        source_index = 0;
    }
    {
        uint8_t c = rb((uint16_t)(dma + source_index++));
        return c == UTIL_EOF ? -1 : c;
    }
}

static int load_hex_digit(void)
{
    int c = load_char();
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

static int load_hex_byte(uint8_t *sum)
{
    int high = load_hex_digit();
    int low = load_hex_digit();
    uint8_t value;
    if (high < 0 || low < 0)
        return -1;
    value = (uint8_t)((high << 4) | low);
    *sum = (uint8_t)(*sum + value);
    return value;
}

static int load_write_byte(uint8_t value, uint8_t *used)
{
    wb((uint16_t)(dma + *used), value);
    (*used)++;
    if (*used == 128) {
        util_set_dma(dma);
        if (util_call(21, dest_fcb) != 0)
            return 1;
        *used = 0;
    }
    return 0;
}

void main(void)
{
    uint8_t length = util_args(args);
    uint8_t pos = 0, current_drive = util_current_drive();
    uint8_t src_drive, dst_drive, used = 0;
    uint16_t output_address = 0x0100;
    uint8_t saw_eof = 0, failed = 0;

    source_fcb = util_fcb_address(0);
    dest_fcb = util_fcb_address(1);
    dma = util_dma_buffer();
    while (pos < length && args[pos] == ' ')
        pos++;
    if (!util_parse_fcb(args, length, &pos, source_fcb,
                        (uint8_t)(current_drive + 1), 0)) {
        util_print("FILE NAME REQUIRED");
        util_print_newline();
        return;
    }
    src_drive = rb(source_fcb) ? (uint8_t)(rb(source_fcb) - 1) :
                               current_drive;
    dst_drive = src_drive;
    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos + 1 < length && args[pos + 1] == ':') {
        uint8_t drive = args[pos];
        if (drive >= 'a' && drive <= 'z')
            drive = (uint8_t)(drive - ('a' - 'A'));
        if (drive < 'A' || drive > 'P') {
            util_print("INVALID DESTINATION DRIVE");
            util_print_newline();
            return;
        }
        dst_drive = (uint8_t)(drive - 'A');
        pos = (uint8_t)(pos + 2);
    }
    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos != length) {
        util_print("INVALID COMMAND TAIL");
        util_print_newline();
        return;
    }
    wb((uint16_t)(source_fcb + 9), 'H');
    wb((uint16_t)(source_fcb + 10), 'E');
    wb((uint16_t)(source_fcb + 11), 'X');
    util_select(src_drive);
    if (util_call(15, source_fcb) == 0xFF) {
        util_print("CANNOT OPEN SOURCE");
        util_print_newline();
        goto done;
    }
    util_clear_fcb(dest_fcb);
    wb(dest_fcb, (uint8_t)(dst_drive + 1));
    {
        uint8_t i;
        for (i = 1; i < 9; i++)
            wb((uint16_t)(dest_fcb + i), rb((uint16_t)(source_fcb + i)));
    }
    wb((uint16_t)(dest_fcb + 9), 'C');
    wb((uint16_t)(dest_fcb + 10), 'O');
    wb((uint16_t)(dest_fcb + 11), 'M');
    util_select(dst_drive);
    (void)util_call(19, dest_fcb);
    if (util_call(22, dest_fcb) == 0xFF) {
        util_print("NO MORE DIRECTORY SPACE");
        util_print_newline();
        util_select(src_drive);
        (void)util_call(16, source_fcb);
        goto done;
    }
    util_select(src_drive);
    source_index = 128;
    for (;;) {
        int c;
        uint8_t sum = 0, count, high, low, type, i;
        do {
            c = load_char();
            if (c < 0) {
                failed = 1;
                goto load_finish;
            }
        } while (c != ':');
        c = load_hex_byte(&sum);
        if (c < 0) {
            failed = 1;
            goto load_finish;
        }
        count = (uint8_t)c;
        c = load_hex_byte(&sum);
        if (c < 0) {
            failed = 1;
            goto load_finish;
        }
        high = (uint8_t)c;
        c = load_hex_byte(&sum);
        if (c < 0) {
            failed = 1;
            goto load_finish;
        }
        low = (uint8_t)c;
        c = load_hex_byte(&sum);
        if (c < 0) {
            failed = 1;
            goto load_finish;
        }
        type = (uint8_t)c;
        if (type == 1) {
            uint8_t checksum;
            if (count != 0 || high != 0 || low != 0) {
                failed = 1;
                goto load_finish;
            }
            c = load_hex_byte(&sum);
            if (c < 0 || sum != 0) {
                failed = 1;
                goto load_finish;
            }
            checksum = (uint8_t)c;
            (void)checksum;
            saw_eof = 1;
            break;
        }
        if (type != 0) {
            failed = 1;
            goto load_finish;
        }
        {
            uint16_t address = (uint16_t)(((uint16_t)high << 8) | low);
            if (address < 0x0100 || (uint32_t)address + count > 0xA000u) {
                failed = 1;
                goto load_finish;
            }
            while (output_address < address) {
                if (load_write_byte(0, &used)) {
                    failed = 2;
                    goto load_finish;
                }
                output_address++;
            }
            if (output_address > address) {
                failed = 1;
                goto load_finish;
            }
            for (i = 0; i < count; i++) {
                c = load_hex_byte(&sum);
                if (c < 0) {
                    failed = 1;
                    goto load_finish;
                }
                if (load_write_byte((uint8_t)c, &used)) {
                    failed = 2;
                    goto load_finish;
                }
                output_address++;
            }
        }
        c = load_hex_byte(&sum);
        if (c < 0 || sum != 0) {
            failed = 1;
            goto load_finish;
        }
    }

load_finish:
    if (!failed && !saw_eof)
        failed = 1;
    if (!failed && used) {
        while (used < 128)
            wb((uint16_t)(dma + used++), UTIL_EOF);
        util_set_dma(dma);
        if (util_call(21, dest_fcb) != 0)
            failed = 2;
    }
    if (!failed && util_call(16, dest_fcb) == 0xFF)
        failed = 2;
    if (!failed) {
        util_print("LOAD COMPLETE");
        util_print_newline();
    } else {
        (void)util_call(16, dest_fcb);
        (void)util_call(19, dest_fcb);
        util_print(failed == 2 ? "DISK OR CLOSE ERROR" :
                   source_read_error ? "DISK READ ERROR" : "INVALID HEX FILE");
        util_print_newline();
    }
    (void)util_call(16, source_fcb);
done:
    util_select(current_drive);
}
