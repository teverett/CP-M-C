/*
 * CP/M-compatible context editor implementation.
 *
 * The editor's text lives in unused TPA memory and disk/console access goes
 * through BDOS, so the same implementation works in hosted, i386, and Z80
 * builds.  Its command language follows the command-oriented interface of
 * the original PL/M ED; the buffer is kept in memory rather than streamed.
 */
#include <stdint.h>
#include "../os/cpm.h"

#define ED_CR       13
#define ED_LF       10
#define ED_EOF      0x1A
#ifdef COM_PROGRAM
#define ED_FCB      0x9E00u
#define ED_TEXT     0x4000u
#define ED_LIMIT    0x9C00u
#else
#define ED_FCB      (CCP_BASE + 0x500)
#define ED_TEXT     0x0100u
#define ED_LIMIT    (CCP_BASE - 0x0200u)
#endif
#define ED_SRC      ED_FCB
#define ED_DST      (ED_FCB + 36)
#define ED_LIB      (ED_FCB + 72)
#define ED_XFER     (ED_FCB + 108)
#define ED_IO       (ED_FCB + 144)
#define ED_CAPACITY (ED_LIMIT - ED_TEXT)

static uint16_t ed_length;
static uint16_t ed_cursor;
static uint16_t ed_line;
static uint16_t ed_io_used;
static uint8_t ed_source_exists;
static uint8_t ed_source_drive;
static uint8_t ed_dest_drive;
static uint8_t ed_upper;
static uint8_t ed_show_lines;
static uint8_t ed_out[128];
static uint8_t ed_macro[128];
static uint8_t ed_macro_len;
static uint16_t ed_macro_repeat;
static uint8_t ed_macro_pos;
static uint8_t ed_macro_active;
static uint8_t ed_transfer_active;
static uint8_t ed_xused;
static uint8_t ed_xout[128];

static void ed_clear_fcb(uint16_t fcb);

static uint8_t ed_text_byte(uint16_t pos)
{
    uint16_t address = (uint16_t)(ED_TEXT + pos);
    return rb(address);
}

static void ed_put(uint8_t c)
{
    (void)bdos_call(2, c);
}

static void ed_name_fcb(uint16_t dst, uint16_t src, const char *ext)
{
    uint8_t i;
    ed_clear_fcb(dst);
    for (i = 0; i < 12; i++)
        wb((uint16_t)(dst + i), rb((uint16_t)(src + i)));
    for (i = 12; i < 36; i++)
        wb((uint16_t)(dst + i), 0);
    if (ext) {
        wb((uint16_t)(dst + 9), (uint8_t)ext[0]);
        wb((uint16_t)(dst + 10), (uint8_t)ext[1]);
        wb((uint16_t)(dst + 11), (uint8_t)ext[2]);
    }
    wb((uint16_t)(dst + 32), 0);
}

static void ed_puts(const char *s)
{
    while (*s)
        ed_put((uint8_t)*s++);
}

static void ed_newline(void)
{
    ed_put(ED_CR);
    ed_put(ED_LF);
}

static uint8_t ed_uppercase(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
        return (uint8_t)(c - ('a' - 'A'));
    return c;
}

static uint8_t ed_digit(uint8_t c)
{
    return (uint8_t)(c >= '0' && c <= '9');
}

static void ed_clear_fcb(uint16_t fcb)
{
    uint8_t i;
    for (i = 0; i < 36; i++)
        wb((uint16_t)(fcb + i), 0);
    for (i = 1; i < 12; i++)
        wb((uint16_t)(fcb + i), ' ');
}

static uint8_t ed_parse_fcb(const uint8_t *s, uint8_t n, uint8_t *p,
                            uint16_t fcb, uint8_t default_drive)
{
    uint8_t i, name = 0, ext = 0, drive = default_drive;
    ed_clear_fcb(fcb);
    if (*p + 1 < n && s[*p + 1] == ':') {
        uint8_t d = ed_uppercase(s[*p]);
        if (d < 'A' || d > 'P')
            return 0;
        drive = (uint8_t)(d - 'A' + 1);
        *p = (uint8_t)(*p + 2);
    }
    if (*p >= n || s[*p] == ' ' || s[*p] == ED_CR || s[*p] == ED_EOF)
        return 0;
    wb(fcb, drive);
    while (*p < n && s[*p] != '.' && s[*p] != ' ' &&
           s[*p] != ED_CR && s[*p] != ED_EOF) {
        if (name < 8)
            wb((uint16_t)(fcb + 1 + name++), ed_uppercase(s[*p]));
        (*p)++;
    }
    if (*p < n && s[*p] == '.') {
        (*p)++;
        while (*p < n && s[*p] != ' ' && s[*p] != ED_CR &&
               s[*p] != ED_EOF) {
            if (ext < 3)
                wb((uint16_t)(fcb + 9 + ext++), ed_uppercase(s[*p]));
            (*p)++;
        }
    }
    for (i = 0; i < 3; i++)
        wb((uint16_t)(fcb + 12 + i), 0);
    wb((uint16_t)(fcb + 32), 0);
    return (uint8_t)(name != 0);
}

static uint8_t ed_select(uint8_t drive)
{
    return (uint8_t)bdos_call(14, drive);
}

static uint8_t ed_setdma(uint16_t address)
{
    return (uint8_t)bdos_call(26, address);
}

static uint8_t ed_read_fcb(uint16_t fcb, uint16_t *length)
{
    uint16_t count = 0;
    uint8_t status;
    ed_setdma(ED_IO);
    for (;;) {
        uint16_t j;
        status = (uint8_t)bdos_call(20, fcb);
        if (status != 0)
            break;
        for (j = 0; j < 128; j++) {
            uint8_t c = rb((uint16_t)(ED_IO + j));
            if (c == ED_EOF) {
                *length = count;
                return 0;
            }
            if (count >= ED_CAPACITY) {
                *length = count;
                return 2;
            }
            wb((uint16_t)(ED_TEXT + count++), ed_upper ? ed_uppercase(c) : c);
        }
    }
    *length = count;
    return (uint8_t)(status == 1 ? 0 : 1);
}

static uint8_t ed_open_source(void)
{
    uint8_t result;
    ed_select(ed_source_drive);
    wb((uint16_t)(ED_SRC + 32), 0);
    result = (uint8_t)bdos_call(15, ED_SRC);
    ed_source_exists = (uint8_t)(result != 0xFF);
    if (!ed_source_exists) {
        ed_length = 0;
        return 0;
    }
    result = ed_read_fcb(ED_SRC, &ed_length);
    return result;
}

static uint16_t ed_line_start(uint16_t pos)
{
    while (pos && rb((uint16_t)(ED_TEXT + pos - 1)) != ED_LF)
        pos--;
    return pos;
}

static uint16_t ed_line_end(uint16_t pos)
{
    while (pos < ed_length && rb((uint16_t)(ED_TEXT + pos)) != ED_LF)
        pos++;
    if (pos < ed_length)
        pos++;
    return pos;
}

static uint16_t ed_line_at(uint16_t pos)
{
    uint16_t n = 1, i;
    for (i = 0; i < pos && i < ed_length; i++)
        if (rb((uint16_t)(ED_TEXT + i)) == ED_LF)
            n++;
    return n;
}

static void ed_print_number(uint16_t value)
{
    uint16_t div = 10000;
    uint8_t started = 0;
    while (div) {
        uint8_t digit = (uint8_t)(value / div);
        value = (uint16_t)(value % div);
        if (digit || started) {
            ed_put((uint8_t)('0' + digit));
            started = 1;
        } else {
            ed_put(' ');
        }
        div = (uint16_t)(div / 10);
    }
}

static void ed_print_line(uint16_t start, uint16_t end)
{
    uint16_t i;
    if (ed_show_lines) {
        ed_print_number(ed_line_at(start));
        ed_put(':');
        ed_put(' ');
        ed_put('*');
    }
    for (i = start; i < end; i++) {
        uint8_t c = rb((uint16_t)(ED_TEXT + i));
        if (c == ED_LF) {
            ed_put(ED_LF);
        } else if (c == ED_CR) {
            ed_put(ED_CR);
        } else if (c == '\t') {
            ed_put(' ');
            ed_put(' ');
            ed_put(' ');
            ed_put(' ');
        } else if (c < ' ') {
            ed_put('^');
            ed_put((uint8_t)(c + '@'));
        } else {
            ed_put(c);
        }
    }
}

static void ed_insert(uint16_t pos, uint8_t c)
{
    uint16_t i;
    if (ed_length >= ED_CAPACITY)
        return;
    for (i = ed_length; i > pos; i--)
        wb((uint16_t)(ED_TEXT + i), rb((uint16_t)(ED_TEXT + i - 1)));
    wb((uint16_t)(ED_TEXT + pos), c);
    ed_length++;
    if (pos <= ed_cursor)
        ed_cursor++;
}

static void ed_delete(uint16_t start, uint16_t end)
{
    uint16_t i, removed;
    if (start > ed_length)
        return;
    if (end > ed_length)
        end = ed_length;
    if (end <= start)
        return;
    removed = (uint16_t)(end - start);
    for (i = start; i + removed < ed_length; i++)
        wb((uint16_t)(ED_TEXT + i), rb((uint16_t)(ED_TEXT + i + removed)));
    ed_length = (uint16_t)(ed_length - removed);
    if (ed_cursor >= end)
        ed_cursor = (uint16_t)(ed_cursor - removed);
    else if (ed_cursor > start)
        ed_cursor = start;
}

static uint8_t ed_write_sector(uint16_t fcb)
{
    uint8_t i, status;
    ed_setdma(ED_IO);
    for (i = 0; i < 128; i++)
        wb((uint16_t)(ED_IO + i), ed_out[i]);
    status = (uint8_t)bdos_call(21, fcb);
    return status;
}

static uint8_t ed_emit(uint16_t fcb, uint8_t c)
{
    ed_out[ed_io_used++] = c;
    if (ed_io_used == 128) {
        ed_io_used = 0;
        return ed_write_sector(fcb);
    }
    return 0;
}

static uint8_t ed_emit_range(uint16_t fcb, uint16_t start, uint16_t end)
{
    uint16_t i;
    for (i = start; i < end; i++)
        if (ed_emit(fcb, rb((uint16_t)(ED_TEXT + i))) != 0)
            return 1;
    return 0;
}

static uint8_t ed_finish_file(void)
{
    uint8_t i, status;
    if (ed_io_used) {
        while (ed_io_used < 128)
            ed_out[ed_io_used++] = ED_EOF;
        ed_io_used = 0;
        status = ed_write_sector(ED_DST);
        if (status != 0)
            return 0;
    }
    status = (uint8_t)bdos_call(16, ED_DST);
    if (status == 0xFF)
        return 0;

    ed_select(ed_source_drive);
    ed_name_fcb(ED_DST, ED_SRC, "BAK");
    (void)bdos_call(19, ED_DST);
    if (ed_source_exists) {
        ed_name_fcb(ED_DST, ED_SRC, 0);
        for (i = 0; i < 16; i++)
            wb((uint16_t)(ED_DST + 16 + i), rb((uint16_t)(ED_DST + i)));
        wb((uint16_t)(ED_DST + 25), 'B');
        wb((uint16_t)(ED_DST + 26), 'A');
        wb((uint16_t)(ED_DST + 27), 'K');
        (void)bdos_call(23, ED_DST);
    }

    ed_name_fcb(ED_DST, ED_SRC, "$$$");
    wb(ED_DST, 0);
    ed_select(ed_dest_drive);
    for (i = 0; i < 16; i++)
        wb((uint16_t)(ED_DST + 16 + i), rb((uint16_t)(ED_SRC + i)));
    status = (uint8_t)bdos_call(23, ED_DST);
    (void)status;
    ed_select(ed_dest_drive);
    wb(4, (uint8_t)((rb(4) & 0xF0) | ed_dest_drive));
    return 1;
}

static uint8_t ed_parse_count(const uint8_t *line, uint8_t len, uint8_t *p,
                              uint16_t *count, uint8_t *direction)
{
    uint16_t n = 0;
    uint8_t have = 0;
    *direction = 1;
    if (*p < len && line[*p] == '-') {
        *direction = 0;
        (*p)++;
    } else if (*p < len && line[*p] == '+') {
        (*p)++;
    }
    if (*p < len && line[*p] == '#') {
        *count = 0xFFFF;
        (*p)++;
        return 1;
    }
    while (*p < len && ed_digit(line[*p])) {
        have = 1;
        n = (uint16_t)(n * 10 + line[*p] - '0');
        (*p)++;
    }
    *count = have ? n : 1;
    return have;
}

static uint16_t ed_move_lines(uint16_t pos, uint16_t count, uint8_t forward)
{
    while (count-- && ((forward && pos < ed_length) || (!forward && pos))) {
        if (forward)
            pos = ed_line_end(pos);
        else
            pos = ed_line_start((uint16_t)(pos - 1));
    }
    return pos;
}

static void ed_type(uint16_t pos, uint16_t count, uint8_t forward)
{
    uint16_t end, start;
    if (forward) {
        start = pos;
        end = ed_move_lines(pos, count, 1);
    } else {
        start = ed_move_lines(pos, count, 0);
        end = pos;
    }
    ed_print_line(start, end);
}

static uint8_t ed_find(const uint8_t *needle, uint8_t nlen, uint16_t from,
                       uint16_t *found)
{
    uint16_t i, j;
    if (!nlen)
        return 0;
    for (i = from; i + nlen <= ed_length; i++) {
        for (j = 0; j < nlen; j++)
            if (rb((uint16_t)(ED_TEXT + i + j)) != needle[j])
                break;
        if (j == nlen) {
            *found = i;
            return 1;
        }
    }
    return 0;
}

static uint8_t ed_insert_input(uint16_t *pos, uint8_t upper)
{
    uint8_t c;
    for (;;) {
        c = (uint8_t)bdos_call(1, 0);
        if (c == ED_EOF)
            break;
        if (c == ED_CR) {
            ed_insert(*pos, ED_CR);
            (*pos)++;
            ed_insert(*pos, ED_LF);
            (*pos)++;
            ed_put(ED_LF);
            if (upper) {
                ed_print_number(ed_line_at(*pos));
                ed_put(':');
                ed_put(' ');
                ed_put('*');
            }
            continue;
        }
        if (upper)
            c = ed_uppercase(c);
        if (c == 8 || c == 0x7F) {
            if (*pos > ed_cursor) {
                ed_delete((uint16_t)(*pos - 1), *pos);
                (*pos)--;
                ed_put(8);
                ed_put(' ');
                ed_put(8);
            }
        } else {
            ed_insert(*pos, c);
            (*pos)++;
        }
        if (c == 0x1B)
            break;
    }
    ed_cursor = *pos;
    return 1;
}

static uint8_t ed_load_library(uint8_t *line, uint8_t len, uint8_t p,
                               uint8_t translate)
{
    uint8_t i;
    uint16_t insert_at = ed_cursor;
    while (p < len && line[p] == ' ')
        p++;
    ed_clear_fcb(ED_LIB);
    wb(ED_LIB, 0);
    for (i = 1; i < 9; i++)
        wb((uint16_t)(ED_LIB + i), ' ');
    for (i = 9; i < 12; i++)
        wb((uint16_t)(ED_LIB + i), ' ');
    if (p >= len) {
        static const char xname[] = "X$$$$$$$LIB";
        for (i = 0; i < 11; i++)
            wb((uint16_t)(ED_LIB + i + 1), (uint8_t)xname[i]);
    } else {
        if (!ed_parse_fcb(line, len, &p, ED_LIB, 0))
            return 0;
        if (rb((uint16_t)(ED_LIB + 9)) == ' ' &&
            rb((uint16_t)(ED_LIB + 10)) == ' ' &&
            rb((uint16_t)(ED_LIB + 11)) == ' ') {
            wb((uint16_t)(ED_LIB + 9), 'L');
            wb((uint16_t)(ED_LIB + 10), 'I');
            wb((uint16_t)(ED_LIB + 11), 'B');
        }
    }
    ed_select((uint8_t)(rb(ED_LIB) ? rb(ED_LIB) - 1 : ed_source_drive));
    wb((uint16_t)(ED_LIB + 32), 0);
    if ((uint8_t)bdos_call(15, ED_LIB) == 0xFF)
        return 0;
    ed_setdma(ED_IO);
    for (;;) {
        uint8_t status = (uint8_t)bdos_call(20, ED_LIB);
        uint16_t j;
        if (status == 1)
            break;
        if (status != 0) {
            (void)bdos_call(16, ED_LIB);
            return 0;
        }
        for (j = 0; j < 128; j++) {
            uint8_t c = rb((uint16_t)(ED_IO + j));
            if (c == ED_EOF)
                goto loaded;
            if (ed_length >= ED_CAPACITY) {
                (void)bdos_call(16, ED_LIB);
                return 0;
            }
            ed_insert(insert_at++, translate ? ed_uppercase(c) : c);
        }
    }
loaded:
    (void)bdos_call(16, ED_LIB);
    ed_cursor = insert_at;
    return 1;
}

static void ed_xfer_name(void)
{
    static const char name[] = "X$$$$$$$LIB";
    uint8_t i;
    ed_clear_fcb(ED_XFER);
    wb(ED_XFER, 0);
    for (i = 0; i < 11; i++)
        wb((uint16_t)(ED_XFER + 1 + i), (uint8_t)name[i]);
    wb((uint16_t)(ED_XFER + 32), 0);
}

static uint8_t ed_xfer_byte(uint8_t c)
{
    uint8_t i, status;
    ed_xout[ed_xused++] = c;
    if (ed_xused != 128)
        return 0;
    for (i = 0; i < 128; i++)
        wb((uint16_t)(ED_IO + i), ed_xout[i]);
    ed_setdma(ED_IO);
    status = (uint8_t)bdos_call(21, ED_XFER);
    ed_xused = 0;
    return status;
}

static void ed_write_prefix(uint16_t end)
{
    if (ed_emit_range(ED_DST, 0, end) != 0) {
        ed_puts("DISK OR DIRECTORY FULL");
        ed_newline();
    } else {
        ed_delete(0, end);
    }
}

static void ed_command_loop(void)
{
    uint8_t command[128];
    uint16_t line_len, pos, count, start, end, found;
    uint8_t len, p, cmd, direction, status, i, has_count, raw_cmd;
    uint8_t translate_data;
    ed_show_lines = 1;
    ed_upper = 0;
    ed_cursor = 0;
    ed_line = 1;
    ed_io_used = 0;
    ed_macro_len = ed_macro_repeat = ed_macro_pos = ed_macro_active = 0;
    ed_transfer_active = 0;
    if (!ed_source_exists) {
        ed_puts("NEW FILE");
        ed_newline();
    }

    for (;;) {
        uint16_t bdosline = ED_IO;
        uint8_t macro_input = ed_macro_active;
        if (macro_input) {
            while (ed_macro_pos >= ed_macro_len) {
                ed_macro_pos = 0;
                if (ed_macro_repeat) {
                    ed_macro_repeat--;
                    if (!ed_macro_repeat) {
                        ed_macro_active = 0;
                        macro_input = 0;
                        break;
                    }
                } else if (ed_cursor >= ed_length) {
                    ed_macro_active = 0;
                    macro_input = 0;
                    break;
                }
            }
        }
        if (macro_input) {
            len = 0;
            while (ed_macro_pos < ed_macro_len &&
                   ed_macro[ed_macro_pos] != ED_EOF &&
                   ed_macro[ed_macro_pos] != ED_CR && len < 127)
                command[len++] = ed_macro[ed_macro_pos++];
            if (ed_macro_pos < ed_macro_len &&
                ed_macro[ed_macro_pos] == ED_EOF)
                ed_macro_pos++;
            else
                ed_macro_pos = ed_macro_len;
            if (!len)
                continue;
        } else {
            if (ed_show_lines) {
                ed_print_number(ed_line_at(ed_cursor));
                ed_put(':');
                ed_put(' ');
                ed_put('*');
            } else {
                ed_put('*');
            }
            wb(bdosline, 128);
            wb((uint16_t)(bdosline + 1), 0);
            (void)bdos_call(10, bdosline);
            len = rb((uint16_t)(bdosline + 1));
            if (len > 127)
                len = 127;
            for (i = 0; i < len; i++)
                command[i] = rb((uint16_t)(bdosline + 2 + i));
            ed_put(ED_LF);
        }
        line_len = 0;
        p = 0;
        while (p < len && command[p] == ' ')
            p++;
        if (p == len) {
            ed_cursor = ed_move_lines(ed_cursor, 1, 1);
            ed_type(ed_cursor, 1, 1);
            continue;
        }
        while (p < len && line_len < 127)
            command[line_len++] = command[p++];
        command[line_len] = 0;
        p = 0;
        has_count = ed_parse_count(command, (uint8_t)line_len, &p, &count,
                                   &direction);
        if (!has_count)
            count = 1;
        if (p >= line_len) {
            cmd = ED_CR;
            translate_data = ed_upper;
        } else {
            raw_cmd = command[p++];
            cmd = ed_uppercase(raw_cmd);
            translate_data = (uint8_t)(ed_upper ||
                (raw_cmd >= 'A' && raw_cmd <= 'Z'));
        }

        if (cmd == 'E') {
            if (ed_emit_range(ED_DST, 0, ed_length) != 0) {
                ed_puts("DISK OR DIRECTORY FULL");
                ed_newline();
                continue;
            }
            if (!ed_finish_file()) {
                ed_puts("DISK OR DIRECTORY FULL");
                ed_newline();
            } else if (ed_transfer_active) {
                ed_select(ed_source_drive);
                (void)bdos_call(16, ED_XFER);
                (void)bdos_call(19, ED_XFER);
            }
            return;
        }
        if (cmd == 'Q') {
            if (line_len == 1 && !ed_macro_active) {
                ed_newline();
                ed_put('Q');
                ed_puts("-(Y/N)");
                status = ed_uppercase((uint8_t)bdos_call(1, 0));
                ed_newline();
                if (status != 'Y')
                    continue;
            }
            (void)bdos_call(16, ED_DST);
            (void)bdos_call(19, ED_DST);
            if (ed_transfer_active) {
                ed_select(ed_source_drive);
                (void)bdos_call(16, ED_XFER);
                (void)bdos_call(19, ED_XFER);
            }
            ed_select(ed_source_drive);
            return;
        }
        if (cmd == 'O') {
            if (line_len == 1 && !ed_macro_active) {
                ed_newline();
                ed_put('O');
                ed_puts("-(Y/N)");
                status = ed_uppercase((uint8_t)bdos_call(1, 0));
                ed_newline();
                if (status != 'Y')
                    continue;
            }
            (void)bdos_call(16, ED_DST);
            (void)bdos_call(19, ED_DST);
            ed_select(ed_source_drive);
            status = ed_open_source();
            if (status) {
                ed_puts("CANNOT READ FILE");
                ed_newline();
                return;
            }
            ed_name_fcb(ED_DST, ED_SRC, "$$$");
            wb(ED_DST, 0);
            ed_select(ed_dest_drive);
            (void)bdos_call(19, ED_DST);
            if ((uint8_t)bdos_call(22, ED_DST) == 0xFF) {
                ed_puts("DISK OR DIRECTORY FULL");
                ed_newline();
                return;
            }
            ed_cursor = 0;
            ed_io_used = 0;
            continue;
        }
        if (cmd == 'I') {
            pos = ed_cursor;
            if (p == line_len) {
                ed_print_number(ed_line_at(pos));
                ed_put(':');
                ed_put(' ');
                ed_put('*');
                ed_newline();
                (void)ed_insert_input(&pos, translate_data);
            } else {
                while (p < line_len) {
                    uint8_t c = command[p++];
                    if (c == ED_EOF)
                        break;
                    if (c == 0x0C) {
                        ed_insert(pos++, ED_CR);
                        ed_insert(pos++, ED_LF);
                    } else {
                        if (translate_data)
                            c = ed_uppercase(c);
                        ed_insert(pos++, c);
                    }
                }
                ed_cursor = pos;
            }
            continue;
        }
        if (cmd == 'R') {
            if (p >= line_len && ed_transfer_active) {
                if (ed_xused) {
                    while (ed_xused && ed_xused < 128)
                        if (ed_xfer_byte(ED_EOF) != 0)
                            break;
                }
                (void)bdos_call(16, ED_XFER);
                ed_transfer_active = 0;
                ed_xused = 0;
            }
            status = ed_load_library(command, (uint8_t)line_len, p,
                                     translate_data);
            ed_select(ed_source_drive);
            if (!status) {
                ed_puts("CANNOT OPEN .LIB");
                ed_newline();
            }
            continue;
        }
        if (cmd == 'H') {
            ed_cursor = 0;
            continue;
        }
        if (cmd == 'A') {
            ed_cursor = ed_length;
            continue;
        }
        if (cmd == 'F' || cmd == 'N') {
            uint8_t needle[100], nlen = 0;
            while (p < line_len && command[p] != ED_EOF && nlen < 100)
                needle[nlen++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            if (p < line_len && command[p] == ED_EOF)
                p++;
            while (count--) {
                if (!ed_find(needle, nlen, ed_cursor, &found)) {
                    ed_put('#');
                    ed_newline();
                    break;
                }
                ed_cursor = (uint16_t)(found + nlen);
            }
            continue;
        }
        if (cmd == 'S') {
            uint8_t needle[100], replacement[100], nlen = 0, rlen = 0;
            while (p < line_len && command[p] != ED_EOF && nlen < 100)
                needle[nlen++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            if (p < line_len && command[p] == ED_EOF)
                p++;
            while (p < line_len && command[p] != ED_EOF && rlen < 100)
                replacement[rlen++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            while (count--) {
                if (!ed_find(needle, nlen, ed_cursor, &found)) {
                    ed_put('#');
                    ed_newline();
                    break;
                }
                ed_delete(found, (uint16_t)(found + nlen));
                for (i = 0; i < rlen; i++)
                    ed_insert((uint16_t)(found + i), replacement[i]);
                ed_cursor = (uint16_t)(found + rlen);
            }
            continue;
        }
        if (cmd == 'M' && !macro_input) {
            if (p < line_len) {
                ed_macro_len = 0;
                while (p < line_len && ed_macro_len < sizeof ed_macro)
                    ed_macro[ed_macro_len++] = translate_data ?
                        ed_uppercase(command[p++]) : command[p++];
                ed_macro_repeat = count > 1 ? count : 0;
                ed_macro_pos = 0;
            } else if (ed_macro_len) {
                ed_macro_active = 1;
            } else {
                ed_put('?');
                ed_newline();
            }
            continue;
        }
        if (cmd == 'B') {
            ed_cursor = direction ? 0 : ed_length;
            continue;
        }
        if (cmd == 'C') {
            if (direction)
                ed_cursor = (uint16_t)((ed_cursor + count > ed_length) ? ed_length : ed_cursor + count);
            else
                ed_cursor = (ed_cursor > count) ? (uint16_t)(ed_cursor - count) : 0;
            continue;
        }
        if (cmd == 'D') {
            if (direction) {
                end = (uint16_t)(ed_cursor + count);
                if (end > ed_length)
                    end = ed_length;
                ed_delete(ed_cursor, end);
            } else {
                start = ed_cursor > count ? (uint16_t)(ed_cursor - count) : 0;
                ed_delete(start, ed_cursor);
                ed_cursor = start;
            }
            continue;
        }
        if (cmd == 'K' || cmd == 'L' || cmd == 'T' || cmd == ED_CR ||
            cmd == 'P') {
            if (cmd == 'P' && (!has_count || count == 0)) {
                direction = 1;
                count = 24;
            }
            if (cmd == 'K') {
                if (direction) {
                    end = ed_move_lines(ed_cursor, count, 1);
                    ed_delete(ed_cursor, end);
                } else {
                    start = ed_move_lines(ed_cursor, count, 0);
                    ed_delete(start, ed_cursor);
                    ed_cursor = start;
                }
            } else if (cmd == 'L') {
                ed_cursor = ed_move_lines(ed_cursor, count, direction);
            } else {
                ed_type(ed_cursor, cmd == ED_CR ? 1 : count, direction);
                if (cmd == ED_CR)
                    ed_cursor = ed_move_lines(ed_cursor, 1, 0);
            }
            continue;
        }
        if (cmd == 'U') {
            ed_upper = direction;
            continue;
        }
        if (cmd == 'V') {
            if (!count) {
                ed_print_number(ed_length);
                ed_put('/');
                ed_print_number(ED_CAPACITY);
                ed_newline();
            } else
                ed_show_lines = direction;
            continue;
        }
        if (cmd == 'W') {
            end = ed_move_lines(0, count, 1);
            ed_write_prefix(end);
            continue;
        }
        if (cmd == 'X') {
            if (count == 0) {
                if (ed_transfer_active) {
                    ed_select(ed_source_drive);
                    (void)bdos_call(16, ED_XFER);
                }
                ed_xfer_name();
                ed_select(ed_source_drive);
                (void)bdos_call(19, ED_XFER);
                ed_transfer_active = 0;
                ed_xused = 0;
            } else {
                if (!ed_transfer_active) {
                    ed_xfer_name();
                    ed_select(ed_source_drive);
                    (void)bdos_call(19, ED_XFER);
                    if ((uint8_t)bdos_call(22, ED_XFER) == 0xFF) {
                        ed_puts("DISK OR DIRECTORY FULL");
                        ed_newline();
                        continue;
                    }
                    ed_transfer_active = 1;
                    ed_xused = 0;
                } else {
                    ed_select(ed_source_drive);
                }
                if (direction) {
                    start = ed_cursor;
                    end = ed_move_lines(ed_cursor, count, 1);
                } else {
                    start = ed_move_lines(ed_cursor, count, 0);
                    end = ed_cursor;
                }
                for (pos = start; pos < end; pos++)
                    if (ed_xfer_byte(ed_text_byte(pos)) != 0) {
                        ed_puts("DISK OR DIRECTORY FULL");
                        ed_newline();
                        break;
                    }
            }
            continue;
        }
        if (cmd == 'Z') {
            if (!count)
                (void)bdos_call(1, 0);
            continue;
        }
        if (cmd == 'J') {
            uint8_t first[100], second[100], third[100];
            uint8_t first_len = 0, second_len = 0, third_len = 0;
            while (p < line_len && command[p] != ED_EOF && first_len < 100)
                first[first_len++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            if (p < line_len && command[p] == ED_EOF)
                p++;
            while (p < line_len && command[p] != ED_EOF && second_len < 100)
                second[second_len++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            if (p < line_len && command[p] == ED_EOF)
                p++;
            while (p < line_len && command[p] != ED_EOF && third_len < 100)
                third[third_len++] = translate_data ?
                    ed_uppercase(command[p++]) : command[p++];
            if (!ed_find(first, first_len, ed_cursor, &found)) {
                ed_put('#');
                ed_newline();
                continue;
            }
            start = (uint16_t)(found + first_len);
            if (!ed_find(third, third_len, start, &end)) {
                ed_put('#');
                ed_newline();
                continue;
            }
            ed_delete(start, end);
            for (i = 0; i < second_len; i++)
                ed_insert((uint16_t)(start + i), second[i]);
            ed_cursor = (uint16_t)(start + second_len);
            continue;
        }
        ed_put('?');
        ed_newline();

    }
}

void ed_command(uint16_t command_buffer, uint8_t command_length,
                uint8_t current_drive)
{
    uint8_t cmd[128], n = 0, p = 0, i, default_drive;
    uint16_t len;
    uint8_t result;

    for (i = 0; i < command_length && n < sizeof cmd; i++)
        cmd[n++] = rb((uint16_t)(command_buffer + i));
    while (p < n && cmd[p] == ' ')
        p++;
    if (n >= 3 && ed_uppercase(cmd[0]) == 'E' &&
        ed_uppercase(cmd[1]) == 'D' && cmd[2] == ' ') {
        while (p < n && cmd[p] != ' ')
            p++;
        while (p < n && cmd[p] == ' ')
            p++;
    }
    if (!ed_parse_fcb(cmd, n, &p, ED_SRC, 0)) {
        ed_puts("FILE NAME REQUIRED");
        ed_newline();
        return;
    }

    default_drive = current_drive;
    ed_upper = 0;
    ed_source_drive = rb(ED_SRC) ? (uint8_t)(rb(ED_SRC) - 1) : default_drive;
    ed_dest_drive = ed_source_drive;
    while (p < n && cmd[p] == ' ')
        p++;
    if (p < n && cmd[p] != ED_CR && cmd[p] != ED_EOF) {
        uint8_t d = ed_uppercase(cmd[p]);
        if (p + 1 < n && cmd[p + 1] == ':' && d >= 'A' && d <= 'P')
            ed_dest_drive = (uint8_t)(d - 'A');
    }
    ed_select(ed_source_drive);
    result = ed_open_source();
    if (result == 2) {
        ed_puts("NO MEMORY");
        ed_newline();
        return;
    }
    if (result) {
        ed_puts("DISK READ ERROR");
        ed_newline();
        return;
    }
    if (ed_source_exists && (rb((uint16_t)(ED_SRC + 9)) & 0x80)) {
        ed_puts("** FILE IS READ/ONLY **");
        ed_newline();
        return;
    }
    if (ed_source_exists && (rb((uint16_t)(ED_SRC + 10)) & 0x80)) {
        ed_puts("\"SYSTEM\" FILE NOT ACCESSIBLE");
        ed_newline();
        return;
    }
    ed_name_fcb(ED_DST, ED_SRC, "$$$");
    wb(ED_DST, 0);
    ed_select(ed_dest_drive);
    if (ed_dest_drive != ed_source_drive &&
        (uint8_t)bdos_call(17, ED_DST) != 0xFF) {
        ed_puts("FILE EXISTS, ERASE IT");
        ed_newline();
        ed_select(ed_source_drive);
        return;
    }
    (void)bdos_call(19, ED_DST);
    if ((uint8_t)bdos_call(22, ED_DST) == 0xFF) {
        ed_puts("DISK OR DIRECTORY FULL");
        ed_newline();
        return;
    }
    ed_io_used = 0;
    len = ed_length;
    (void)len;
    ed_command_loop();
}

#ifdef COM_PROGRAM
void main(void)
{
    uint8_t length = rb(0x0080);
    ed_command(0x0081, length, (uint8_t)bdos_call(25, 0));
}
#endif
