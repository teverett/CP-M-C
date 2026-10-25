#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t args[129];

void main(void)
{
    uint8_t length = util_args(args);
    uint8_t pos = 0, drive, current_drive = util_current_drive();
    uint8_t status, index = 128;
    uint16_t fcb = util_fcb_address(0), dma = util_dma_buffer();
    uint16_t offset = 0;

    while (pos < length && args[pos] == ' ')
        pos++;
    if (!util_parse_fcb(args, length, &pos, fcb,
                        (uint8_t)(current_drive + 1), 0) ||
        (pos < length && args[pos] != ' ')) {
        util_print("FILE NAME REQUIRED");
        util_print_newline();
        return;
    }
    drive = rb(fcb) ? (uint8_t)(rb(fcb) - 1) : current_drive;
    util_select(drive);
    if (util_call(15, fcb) == 0xFF) {
        util_print("NO INPUT FILE PRESENT ON DISK");
        util_print_newline();
        util_select(current_drive);
        return;
    }
    wb((uint16_t)(fcb + 32), 0);
    util_set_dma(dma);
    for (;;) {
        if (index >= 128) {
            status = util_call(20, fcb);
            if (status != 0)
                break;
            index = 0;
        }
        if ((offset & 15) == 0) {
            util_print_newline();
            util_print_hex((uint8_t)(offset >> 8));
            util_print_hex((uint8_t)offset);
        }
        util_print_char(' ');
        util_print_hex(rb((uint16_t)(dma + index++)));
        offset++;
        if (util_call(11, 0) != 0) {
            (void)util_call(1, 0);
            break;
        }
    }
    (void)util_call(16, fcb);
    util_print_newline();
    util_select(current_drive);
}
