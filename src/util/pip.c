#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t args[129];
static uint8_t destination_pattern[11];
static uint8_t matches[128][11];

static uint8_t same_name(const uint8_t *left, const uint8_t *right)
{
    uint8_t i;
    for (i = 0; i < 11; i++)
        if ((left[i] & 0x7F) != (right[i] & 0x7F))
            return 0;
    return 1;
}

static uint8_t remember_match(uint16_t entry, uint8_t count)
{
    uint8_t i, j;
    for (i = 0; i < count; i++) {
        for (j = 0; j < 11; j++)
            if (matches[i][j] !=
                (uint8_t)(rb((uint16_t)(entry + 1 + j)) & 0x7F))
                break;
        if (j == 11)
            return count;
    }
    if (count >= 128)
        return count;
    for (j = 0; j < 11; j++)
        matches[count][j] = (uint8_t)(rb((uint16_t)(entry + 1 + j)) & 0x7F);
    return (uint8_t)(count + 1);
}

static void fill_fcb(uint16_t fcb, uint8_t drive, const uint8_t *name)
{
    uint8_t i;
    util_clear_fcb(fcb);
    wb(fcb, (uint8_t)(drive + 1));
    for (i = 0; i < 11; i++)
        wb((uint16_t)(fcb + 1 + i), name[i]);
}

static uint8_t copy_file(const uint8_t *name, uint8_t source_drive,
                         uint8_t dest_drive, uint16_t source_fcb,
                         uint16_t dest_fcb, uint16_t dma)
{
    uint8_t i, result;
    uint8_t dest_name[11];

    for (i = 0; i < 11; i++)
        dest_name[i] = destination_pattern[i] == '?' ?
                       name[i] : destination_pattern[i];
    if (source_drive == dest_drive && same_name(name, dest_name)) {
        util_print("SOURCE AND DESTINATION ARE THE SAME");
        util_print_newline();
        return 0;
    }
    fill_fcb(source_fcb, source_drive, name);
    util_select(source_drive);
    if (util_call(15, source_fcb) == 0xFF) {
        util_print("SOURCE FILE NOT FOUND");
        util_print_newline();
        return 0;
    }
    fill_fcb(dest_fcb, dest_drive, dest_name);
    util_select(dest_drive);
    (void)util_call(19, dest_fcb);
    if (util_call(22, dest_fcb) == 0xFF) {
        util_print("DESTINATION DIRECTORY FULL");
        util_print_newline();
        util_select(source_drive);
        (void)util_call(16, source_fcb);
        return 0;
    }

    for (;;) {
        util_select(source_drive);
        util_set_dma(dma);
        result = util_call(20, source_fcb);
        if (result == 1)
            break;
        if (result != 0) {
            util_print("DISK READ ERROR");
            util_print_newline();
            util_select(dest_drive);
            (void)util_call(16, dest_fcb);
            (void)util_call(19, dest_fcb);
            util_select(source_drive);
            (void)util_call(16, source_fcb);
            return 0;
        }
        util_select(dest_drive);
        util_set_dma(dma);
        if (util_call(21, dest_fcb) != 0) {
            util_print("DISK WRITE ERROR");
            util_print_newline();
            (void)util_call(16, dest_fcb);
            (void)util_call(19, dest_fcb);
            util_select(source_drive);
            (void)util_call(16, source_fcb);
            return 0;
        }
    }
    util_select(dest_drive);
    if (util_call(16, dest_fcb) == 0xFF) {
        util_print("DESTINATION CLOSE ERROR");
        util_print_newline();
        (void)util_call(19, dest_fcb);
        util_select(source_drive);
        (void)util_call(16, source_fcb);
        return 0;
    }
    util_select(source_drive);
    (void)util_call(16, source_fcb);
    util_print("COPY COMPLETE");
    util_print_newline();
    return 1;
}

void main(void)
{
    uint8_t length = util_args(args);
    uint8_t pos = 0, current_drive, source_drive, dest_drive;
    uint8_t count = 0, found, i, copied = 0;
    uint16_t dest_fcb = util_fcb_address(0);
    uint16_t source_fcb = util_fcb_address(1);
    uint16_t dma = util_dma_buffer();

    current_drive = util_current_drive();
    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos + 1 < length && args[pos + 1] == ':' &&
        (pos + 2 == length || args[pos + 2] == ' ' ||
         args[pos + 2] == '=')) {
        uint8_t letter = args[pos];
        if (letter >= 'a' && letter <= 'z')
            letter = (uint8_t)(letter - ('a' - 'A'));
        if (letter < 'A' || letter > 'P') {
            util_print("INVALID DESTINATION DRIVE");
            util_print_newline();
            return;
        }
        dest_drive = (uint8_t)(letter - 'A');
        util_clear_fcb(dest_fcb);
        wb(dest_fcb, (uint8_t)(dest_drive + 1));
        for (i = 0; i < 11; i++)
            destination_pattern[i] = '?';
        pos = (uint8_t)(pos + 2);
    } else {
        if (!util_parse_fcb(args, length, &pos, dest_fcb,
                            (uint8_t)(current_drive + 1), 1)) {
            util_print("PIP DEST=SOURCE");
            util_print_newline();
            return;
        }
        dest_drive = rb(dest_fcb) ? (uint8_t)(rb(dest_fcb) - 1) :
                                   current_drive;
        for (i = 0; i < 11; i++)
            destination_pattern[i] =
                (uint8_t)(rb((uint16_t)(dest_fcb + 1 + i)) & 0x7F);
    }
    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos >= length || args[pos] != '=') {
        util_print("EQUAL SIGN REQUIRED");
        util_print_newline();
        return;
    }
    pos++;
    while (pos < length && args[pos] == ' ')
        pos++;
    if (!util_parse_fcb(args, length, &pos, source_fcb,
                        (uint8_t)(current_drive + 1), 1)) {
        util_print("SOURCE FILE REQUIRED");
        util_print_newline();
        return;
    }
    while (pos < length && args[pos] == ' ')
        pos++;
    if (pos != length) {
        util_print("INVALID COMMAND TAIL");
        util_print_newline();
        return;
    }
    source_drive = rb(source_fcb) ? (uint8_t)(rb(source_fcb) - 1) :
                                  current_drive;

    util_select(source_drive);
    util_set_dma(dma);
    found = util_call(17, source_fcb);
    while (found != 0xFF) {
        uint16_t entry = (uint16_t)(dma + ((found & 3u) * 32u));
        if (rb(entry) != 0xE5)
            count = remember_match(entry, count);
        found = util_call(18, 0);
    }
    if (!count) {
        util_print("SOURCE FILE NOT FOUND");
        util_print_newline();
        goto done;
    }
    for (i = 0; i < count; i++) {
        if (!copy_file(matches[i], source_drive, dest_drive,
                       source_fcb, dest_fcb, dma))
            goto done;
        copied++;
    }
    if (copied > 1) {
        util_print_decimal(copied);
        util_print(" FILES COPIED");
        util_print_newline();
    }
done:
    util_select(current_drive);
}
