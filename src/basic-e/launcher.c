#include <stdint.h>
#include "../os/cpm.h"
#include "../util/util.h"

void basic_overlay(void);

void main(void)
{
    uint8_t args[128], length, pos = 0, i;
    uint16_t fcb = util_fcb_address(0);
    const char *runtime;
    const char *error = "ERROR: Cannot load BASIC runtime\r\n$";

    length = util_args(args);
    while (pos < length && args[pos] == ' ')
        ++pos;
    if (!util_parse_fcb(args, length, &pos, fcb, 0, 0)) {
        util_print("ERROR: Usage: BASIC filename[.BAS|.INT]\r\n");
        return;
    }
    while (pos < length && args[pos] == ' ')
        ++pos;
    if (pos != length) {
        util_print("ERROR: Unexpected command arguments\r\n");
        return;
    }
    if (rb(fcb + 9) == 'I' && rb(fcb + 10) == 'N' &&
        rb(fcb + 11) == 'T')
        runtime = "BASICINTCOM";
    else if (rb(fcb + 9) == ' ' ||
             (rb(fcb + 9) == 'B' && rb(fcb + 10) == 'A' &&
              rb(fcb + 11) == 'S'))
        runtime = "BASICBASCOM";
    else {
        util_print("ERROR: Expected .BAS or .INT source\r\n");
        return;
    }
    /* The overlay must not depend on the drive containing the source file. */
    for (i = 0; error[i]; ++i)
        wb(0x9E00 + i, (uint8_t)error[i]);
    util_clear_fcb(0x9D00);
    for (i = 0; i < 11; ++i)
        wb(0x9D01 + i, (uint8_t)runtime[i]);
    if (util_call(15, 0x9D00) == 255) {
        wb(0x9D00, 1);
        if (util_call(15, 0x9D00) == 255) {
            util_print("ERROR: Missing BASIC runtime on current drive or A:\r\n");
            return;
        }
    }
    basic_overlay();
}
