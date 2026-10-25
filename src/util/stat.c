#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t args[129];
static uint8_t names[128][11];
static uint8_t attributes[128];
static uint16_t records[128];

static void disk_status(uint8_t drive)
{
    uint16_t dpb, allocation, blocks, free_blocks = 0, i;
    uint8_t shift;

    util_select(drive);
    dpb = util_call_pointer(31, 0);
    allocation = util_call_pointer(27, 0);
    if (!dpb || !allocation) {
        util_print("DISK PARAMETERS UNAVAILABLE");
        util_print_newline();
        return;
    }
    blocks = (uint16_t)(rw((uint16_t)(dpb + 5)) + 1);
    for (i = 0; i < blocks; i++) {
        if ((rb((uint16_t)(allocation + (i >> 3))) &
             (uint8_t)(1u << (i & 7))) == 0)
            free_blocks++;
    }
    shift = rb((uint16_t)(dpb + 2));
    util_print("DRIVE ");
    util_print_char((uint8_t)('A' + drive));
    util_print(": ");
    util_print_decimal(free_blocks);
    util_print(" FREE BLOCKS OF ");
    util_print_decimal(blocks);
    util_print_newline();
    util_print("BLOCK SIZE ");
    util_print_decimal((uint16_t)(128u << shift));
    util_print(" BYTES");
    util_print_newline();
}

static void list_files(uint16_t fcb)
{
    uint16_t dma = util_dma_buffer();
    uint8_t found, count = 0, i;

    util_set_dma(dma);
    found = util_call(17, fcb);
    while (found != 0xFF) {
        uint16_t entry = (uint16_t)(dma + ((found & 3u) * 32u));
        uint8_t user = rb(entry);
        if (user != 0xE5) {
            uint8_t index, j;
            for (index = 0; index < count; index++) {
                for (j = 0; j < 11; j++)
                    if (names[index][j] !=
                        (uint8_t)(rb((uint16_t)(entry + 1 + j)) & 0x7F))
                        break;
                if (j == 11)
                    break;
            }
            if (index == count && count < 128) {
                for (j = 0; j < 11; j++)
                    names[count][j] =
                        (uint8_t)(rb((uint16_t)(entry + 1 + j)) & 0x7F);
                attributes[count] = 0;
                records[count] = 0;
                count++;
            }
            if (index < count) {
                records[index] = (uint16_t)(records[index] +
                                             rb((uint16_t)(entry + 15)));
                attributes[index] |=
                    (uint8_t)(((rb((uint16_t)(entry + 9)) & 0x80) ? 1 : 0) |
                              ((rb((uint16_t)(entry + 10)) & 0x80) ? 2 : 0));
            }
        }
        found = util_call(18, 0);
    }
    if (!count) {
        util_print("NO FILES FOUND");
        util_print_newline();
        return;
    }
    for (i = 0; i < count; i++) {
        uint8_t j;
        util_clear_fcb(fcb);
        for (j = 0; j < 11; j++)
            wb((uint16_t)(fcb + 1 + j), names[i][j]);
        util_print_filename(fcb);
        util_print_char(' ');
        util_print_decimal(records[i]);
        util_print(" REC");
        if (attributes[i] & 1)
            util_print(" R/O");
        if (attributes[i] & 2)
            util_print(" SYS");
        util_print_newline();
    }
}

void main(void)
{
    uint8_t length = util_args(args);
    uint8_t current_drive = util_current_drive();
    uint8_t pos = 0, drive = current_drive;
    uint16_t fcb = util_fcb_address(0);

    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos == length) {
        util_clear_fcb(fcb);
        wb(fcb, (uint8_t)(drive + 1));
        {
            uint8_t i;
            for (i = 1; i < 12; i++)
                wb((uint16_t)(fcb + i), '?');
        }
        list_files(fcb);
        util_select(current_drive);
        return;
    }
    if (length - pos >= 4 &&
        (args[pos] == 'D' || args[pos] == 'd') &&
        (args[pos + 1] == 'S' || args[pos + 1] == 's') &&
        (args[pos + 2] == 'K' || args[pos + 2] == 'k') &&
        args[pos + 3] == ':') {
        disk_status(drive);
        util_select(current_drive);
        return;
    }
    if (pos + 1 < length && args[pos + 1] == ':' &&
        (pos + 2 == length || args[pos + 2] == ' ')) {
        uint8_t letter = args[pos];
        if (letter >= 'a' && letter <= 'z')
            letter = (uint8_t)(letter - ('a' - 'A'));
        if (letter < 'A' || letter > 'P') {
            util_print("INVALID DRIVE");
            util_print_newline();
            return;
        }
        drive = (uint8_t)(letter - 'A');
        util_clear_fcb(fcb);
        wb(fcb, (uint8_t)(drive + 1));
        {
            uint8_t i;
            for (i = 1; i < 12; i++)
                wb((uint16_t)(fcb + i), '?');
        }
        list_files(fcb);
        util_select(current_drive);
        return;
    }
    if (!util_parse_fcb(args, length, &pos, fcb,
                        (uint8_t)(drive + 1), 1)) {
        util_print("INVALID FILE SPECIFICATION");
        util_print_newline();
        return;
    }
    drive = rb(fcb) ? (uint8_t)(rb(fcb) - 1) : current_drive;
    util_select(drive);
    list_files(fcb);
    util_select(current_drive);
}
