#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

static uint8_t args[129];
static uint8_t line[126];
static uint8_t commands[2048];
static uint16_t offsets[1024];
static uint16_t source_fcb, submit_fcb, dma;
static uint16_t source_index;
static uint16_t command_bytes;
static uint16_t command_count;
static uint8_t source_drive;
static uint8_t source_read_error;

static int source_char(void)
{
    uint8_t c;
    if (source_index >= 128) {
        util_select(source_drive);
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
    c = rb((uint16_t)(dma + source_index++));
    if (c == UTIL_EOF)
        return -1;
    if (c >= 'a' && c <= 'z')
        c = (uint8_t)(c - ('a' - 'A'));
    return c;
}

static uint8_t add_command(uint8_t length)
{
    uint8_t i;
    if (command_count >= 1024 ||
        (uint16_t)(command_bytes + length + 1) > sizeof commands)
        return 0;
    offsets[command_count++] = command_bytes;
    commands[command_bytes++] = length;
    for (i = 0; i < length; i++)
        commands[command_bytes++] = line[i];
    return 1;
}

static uint8_t add_expanded(uint8_t value, uint8_t *length)
{
    if (*length >= 125)
        return 0;
    line[(*length)++] = value;
    return 1;
}

static uint8_t parameter(uint8_t index, uint8_t *position,
                         uint8_t length, uint8_t *out_length)
{
    uint8_t start, end, skipped = 0;
    while (*position < length && args[*position] == ' ')
        (*position)++;
    while (*position < length) {
        start = *position;
        while (*position < length && args[*position] != ' ')
            (*position)++;
        end = *position;
        if (skipped++ == index) {
            uint8_t n = (uint8_t)(end - start), i;
            for (i = 0; i < n; i++)
                if (!add_expanded(args[start + i], out_length))
                    return 0;
            return 1;
        }
        while (*position < length && args[*position] == ' ')
            (*position)++;
    }
    return 1;
}

static uint8_t expand_line(uint8_t *length, uint8_t arg_pos, uint8_t arg_len)
{
    int c;
    *length = 0;
    while ((c = source_char()) >= 0 && c != UTIL_CR) {
        if (c == UTIL_LF)
            continue;
        if (c == '$') {
            int next = source_char();
            if (next == '$') {
                if (!add_expanded('$', length))
                    return 0;
            } else {
                uint8_t parameter_pos = arg_pos, index;
                if (next < '0' || next > '9')
                    return 0;
                index = (uint8_t)(next - '0');
                if (!parameter(index, &parameter_pos, arg_len, length))
                    return 0;
            }
        } else if (c == '^') {
            int control = source_char();
            if (control < 'A' || control > 'Z' ||
                !add_expanded((uint8_t)(control - 'A' + 1), length))
                return 0;
        } else if (!add_expanded((uint8_t)c, length)) {
            return 0;
        }
    }
    return (uint8_t)(c == UTIL_CR ? 1 : 2);
}

static void report(const char *message)
{
    util_print(message);
    util_print_newline();
}

void main(void)
{
    uint8_t length = util_args(args), pos = 0, current_drive;
    uint8_t arg_pos, source_status, line_length, line_result;

    current_drive = util_current_drive();
    source_fcb = util_fcb_address(0);
    submit_fcb = util_fcb_address(1);
    dma = util_dma_buffer();
    while (pos < length && args[pos] == ' ')
        pos++;
    if (!util_parse_fcb(args, length, &pos, source_fcb,
                        (uint8_t)(current_drive + 1), 0)) {
        report("SUBMIT FILE.SUB [PARAMETERS]");
        return;
    }
    if (rb((uint16_t)(source_fcb + 9)) == ' ') {
        wb((uint16_t)(source_fcb + 9), 'S');
        wb((uint16_t)(source_fcb + 10), 'U');
        wb((uint16_t)(source_fcb + 11), 'B');
    }
    arg_pos = pos;
    source_drive = rb(source_fcb) ? (uint8_t)(rb(source_fcb) - 1) :
                                   current_drive;
    util_select(source_drive);
    if (util_call(15, source_fcb) == 0xFF) {
        report("SUB FILE NOT FOUND");
        goto finish;
    }
    source_index = 128;
    do {
        line_result = expand_line(&line_length, arg_pos, length);
        if (line_result == 0) {
            report(source_read_error ? "SUBMIT DISK READ ERROR" :
                                      "SUBMIT EXPANSION ERROR");
            (void)util_call(16, source_fcb);
            goto finish;
        }
        if (source_read_error) {
            report("SUBMIT DISK READ ERROR");
            (void)util_call(16, source_fcb);
            goto finish;
        }
        if (!add_command(line_length)) {
            report("SUBMIT FILE TOO LARGE");
            (void)util_call(16, source_fcb);
            goto finish;
        }
    } while (line_result == 1);
    source_status = util_call(16, source_fcb);
    if (source_status == 0xFF) {
        report("CANNOT CLOSE SUB FILE");
        goto finish;
    }

    util_clear_fcb(submit_fcb);
    wb((uint16_t)(submit_fcb + 1), '$');
    wb((uint16_t)(submit_fcb + 2), '$');
    wb((uint16_t)(submit_fcb + 3), '$');
    wb((uint16_t)(submit_fcb + 9), 'S');
    wb((uint16_t)(submit_fcb + 10), 'U');
    wb((uint16_t)(submit_fcb + 11), 'B');
    util_select(0);
    (void)util_call(19, submit_fcb);
    if (util_call(22, submit_fcb) == 0xFF) {
        report("SUBMIT DIRECTORY FULL");
        goto finish;
    }
    while (command_count) {
        uint16_t offset = offsets[--command_count];
        uint8_t command_length = commands[offset], i;
        for (i = 0; i < 128; i++)
            wb((uint16_t)(dma + i), UTIL_EOF);
        wb(dma, command_length);
        for (i = 0; i < command_length; i++)
            wb((uint16_t)(dma + 1 + i), commands[offset + 1 + i]);
        wb((uint16_t)(dma + command_length + 1), 0);
        wb((uint16_t)(dma + command_length + 2), '$');
        util_set_dma(dma);
        if (util_call(21, submit_fcb) != 0) {
            (void)util_call(16, submit_fcb);
            (void)util_call(19, submit_fcb);
            report("SUBMIT DISK WRITE ERROR");
            goto finish;
        }
    }
    if (util_call(16, submit_fcb) == 0xFF) {
        (void)util_call(19, submit_fcb);
        report("SUBMIT CLOSE ERROR");
        goto finish;
    }
    util_select(current_drive);
    (void)bdos_call(0, 0);
finish:
    util_select(current_drive);
}
