#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../os/cpm.h"
#include "../util/util.h"

#define INT_CODE_MAX 2048u
#define INT_DATA_MAX 256u
#define INT_CONST_MAX 64u
#define INT_CELL_MAX 64u
#define INT_STACK_MAX 32u
#define INT_STRING_MAX 384u
#define INT_FILES 2u
#define INT_RECORD 128u
#define INT_LINE 132u
#define NONE 0xffffu

enum { V_NUMBER, V_STRING, V_ADDRESS, V_ARRAY };
enum { S_CODE, S_HEAP };

typedef struct {
    float number;
    uint16_t value;
    uint8_t kind;
    uint8_t string_kind;
} Value;

typedef struct {
    Value value;
    uint16_t length;
    uint16_t base;
    uint16_t dimensions[4];
    uint8_t rank;
    uint8_t used;
} Array;

typedef struct {
    uint8_t fcb[36];
    uint8_t record[INT_RECORD];
    uint8_t used;
    uint8_t write_mode;
    uint8_t dirty;
    uint8_t record_pos;
    uint8_t record_len;
} IntFile;

static uint8_t code[INT_CODE_MAX];
static uint8_t data[INT_DATA_MAX];
static float constants[INT_CONST_MAX];
static Value cells[INT_CELL_MAX];
static Value stack[INT_STACK_MAX];
static Array arrays[32];
static uint8_t strings[INT_STRING_MAX];
static uint8_t int_dma[INT_RECORD];
static uint8_t int_fcb[36];
static uint8_t int_args[128];
static uint8_t input_buffer[130];
static uint8_t print_buffer[INT_LINE];
static uint8_t field_buffer[128];
static Value field_result;
static IntFile files[INT_FILES];
static uint16_t code_size, data_size, constant_count, cell_count;
static uint16_t pc, sp, data_pos, string_used;
static uint16_t returns[16];
static uint16_t return_sp[16];
static uint8_t return_depth, file_count, print_pos, print_zone;
static uint8_t input_pos, input_len, input_eol, halted, current_line;
static uint8_t int_read_pos, int_read_len, int_eof;
static uint16_t int_record_number;
static uint8_t active_file;
static uint16_t random_seed;

static void dump_console(void);
static void file_read_record(uint8_t slot);
static uint8_t file_get_char(uint8_t slot);

static void fail(const char *message)
{
    if (!halted) {
        util_print("\r\nBASIC-E INT ERROR: ");
        util_print(message);
        if (current_line) {
            util_print(" in line ");
            util_print_decimal(current_line);
        }
        util_print(" at code ");
        util_print_decimal(pc);
        util_print(", stack ");
        util_print_decimal(sp);
        util_print("/");
        util_print_decimal((uint16_t)stack[sp].number);
        util_print_newline();
    }
    halted = 1;
}

static void push(Value value)
{
    if (sp + 1 >= INT_STACK_MAX) {
        fail("expression stack overflow");
        return;
    }
    stack[++sp] = value;
}

static Value pop(void)
{
    Value value;
    value.number = 0;
    value.value = 0;
    value.kind = V_NUMBER;
    value.string_kind = S_HEAP;
    if (!sp) {
        fail("expression stack underflow");
        return value;
    }
    value = stack[sp--];
    return value;
}

static Value zero_value(void)
{
    Value value;
    value.number = 0;
    value.value = 0;
    value.kind = V_NUMBER;
    value.string_kind = S_HEAP;
    return value;
}

static Value number_value(float n)
{
    Value value = zero_value();
    value.number = n;
    return value;
}

static Value address_value(uint16_t n)
{
    Value value = zero_value();
    value.kind = V_ADDRESS;
    value.value = n;
    return value;
}

static Value string_value(uint16_t n, uint8_t kind)
{
    Value value = zero_value();
    value.kind = V_STRING;
    value.value = n;
    value.string_kind = kind;
    return value;
}

static float as_number(Value value)
{
    if (value.kind == V_STRING || value.kind == V_ARRAY) {
        fail("numeric value required");
        return 0;
    }
    if (value.kind == V_ADDRESS)
        return (float)value.value;
    return value.number;
}

static uint16_t as_integer(Value value)
{
    float n = as_number(value);
    if (n < 0) return (uint16_t)(int16_t)n;
    return (uint16_t)n;
}

static uint16_t cell_index(Value value)
{
    uint16_t index = as_integer(value);
    if (index >= cell_count) {
        fail("variable address outside PRT");
        return 0;
    }
    return index;
}

static uint16_t string_length(Value value)
{
    if (value.kind != V_STRING) return 0;
    if (value.string_kind == S_CODE) {
        if (value.value >= code_size) return 0;
        return code[value.value];
    }
    if (value.value >= string_used) return 0;
    return strings[value.value];
}

static uint8_t string_char(Value value, uint16_t index)
{
    if (index >= string_length(value)) return 0;
    if (value.string_kind == S_CODE)
        return code[value.value + 1 + index];
    return strings[value.value + 1 + index];
}

static Value make_string(const uint8_t *source, uint16_t length)
{
    uint16_t start;
    Value result;
    if (length > 255 || string_used + length + 1 > INT_STRING_MAX) {
        fail("string storage exhausted");
        result = zero_value();
    } else {
        start = string_used;
        strings[string_used++] = (uint8_t)length;
        while (length--) strings[string_used++] = *source++;
        result = zero_value();
        result.kind = V_STRING;
        result.value = start;
        result.string_kind = S_HEAP;
    }
    return result;
}

static void copy_string(Value value, uint8_t *dest, uint16_t *length)
{
    uint16_t n = string_length(value), i;
    for (i = 0; i < n; ++i) dest[i] = string_char(value, i);
    *length = n;
}

static uint8_t next_int_byte(void)
{
    uint8_t status;
    if (int_read_pos == int_read_len) {
        if (int_eof) return 0x7f;
        util_set_dma(util_pointer(int_dma));
        status = util_call(20, util_pointer(int_fcb));
        if (status) {
            int_eof = 1;
            return 0x7f;
        }
        int_read_pos = 0;
        int_read_len = INT_RECORD;
        ++int_record_number;
    }
    return int_dma[int_read_pos++];
}

static uint8_t read_int_file(void)
{
    uint8_t length = util_args(int_args), pos = 0;
    uint8_t status;
    uint16_t declared_code, declared_data, declared_cells;
    while (pos < length && int_args[pos] == ' ') ++pos;
    if (!util_parse_fcb(int_args, length, &pos, util_pointer(int_fcb),
                        util_current_drive(), 0)) {
        util_print("BASICINT: ERROR: expected an .INT filename\r\n");
        return 0;
    }
    if (util_call(15, util_pointer(int_fcb)) == 0xff) {
        util_print("BASICINT: ERROR: cannot open .INT file\r\n");
        return 0;
    }
    int_read_pos = int_read_len = int_eof = 0;
    int_record_number = 0;
    for (;;) {
        char text[32];
        uint8_t n = 0, ch;
        float value = 0, fraction = 0.1f;
        uint8_t negative = 0, decimal = 0, exponent = 0, exp_negative = 0;
        uint8_t exp_seen = 0;
        ch = next_int_byte();
        if (ch == '*') break;
        if (ch == 0x7f) {
            util_call(16, util_pointer(int_fcb));
            util_print("BASICINT: ERROR: truncated constant table\r\n");
            return 0;
        }
        text[n++] = (char)ch;
        do {
            ch = next_int_byte();
            if (ch == '$' || ch == 0x7f) break;
            if (n < sizeof(text) - 1) text[n++] = (char)ch;
        } while (1);
        text[n] = 0;
        if (ch == 0x7f) {
            util_call(16, util_pointer(int_fcb));
            util_print("BASICINT: ERROR: truncated constant table\r\n");
            return 0;
        }
        {
            uint8_t i = 0;
            if (text[i] == '-') { negative = 1; ++i; }
            else if (text[i] == '+') ++i;
            for (; text[i] && text[i] != 'E' && text[i] != 'e'; ++i) {
                if (text[i] == '.') { decimal = 1; continue; }
                if (text[i] < '0' || text[i] > '9') continue;
                if (!decimal) value = value * 10.0f + (text[i] - '0');
                else { value += (text[i] - '0') * fraction; fraction *= 0.1f; }
            }
            if (text[i] == 'E' || text[i] == 'e') {
                ++i;
                if (text[i] == '-') { exp_negative = 1; ++i; }
                else if (text[i] == '+') ++i;
                while (text[i] >= '0' && text[i] <= '9') {
                    exponent = (uint8_t)(exponent * 10 + text[i++] - '0');
                    exp_seen = 1;
                }
            }
        }
        if (negative) value = -value;
        if (exp_seen) {
            while (exponent--) {
                if (exp_negative) value *= 0.1f;
                else value *= 10.0f;
            }
        }
        if (constant_count == INT_CONST_MAX) {
            util_call(16, util_pointer(int_fcb));
            util_print("BASICINT: ERROR: constant table exceeds runtime limit\r\n");
            return 0;
        }
        constants[constant_count++] = value;
    }

    declared_code = (uint16_t)((uint16_t)next_int_byte() << 8);
    declared_code |= next_int_byte();
    declared_data = (uint16_t)((uint16_t)next_int_byte() << 8);
    declared_data |= next_int_byte();
    declared_cells = (uint16_t)((uint16_t)next_int_byte() << 8);
    declared_cells |= next_int_byte();
    if (declared_code > INT_CODE_MAX || declared_data > INT_DATA_MAX ||
        declared_cells > INT_CELL_MAX) {
        util_call(16, util_pointer(int_fcb));
        util_print("BASICINT: ERROR: declared program size exceeds runtime limit\r\n");
        return 0;
    }
    cell_count = declared_cells;

    for (;;) {
        uint8_t op = next_int_byte();
        if (op == 0x7f) break;
        if (op == 51) {
            uint8_t ch;
            while ((ch = next_int_byte()) != 0) {
                if (data_size == INT_DATA_MAX) {
                    util_call(16, util_pointer(int_fcb));
                    util_print("BASICINT: ERROR: DATA exceeds runtime limit\r\n");
                    return 0;
                }
                data[data_size++] = ch;
            }
            continue;
        }
        if (code_size == INT_CODE_MAX) {
            util_call(16, util_pointer(int_fcb));
            util_print("BASICINT: ERROR: code exceeds runtime limit\r\n");
            return 0;
        }
        code[code_size++] = op;
        if (op >= 128) {
            uint8_t lo = next_int_byte();
            uint16_t literal = (uint16_t)(((uint16_t)(op & 0x3f) << 8) | lo);
            if (code_size + 2 > INT_CODE_MAX) {
                util_call(16, util_pointer(int_fcb));
                util_print("BASICINT: ERROR: code exceeds runtime limit\r\n");
                return 0;
            }
            code[code_size++] = lo;
            if ((op & 0x40) && literal >= cell_count) {
                if (literal >= INT_CELL_MAX) {
                    util_call(16, util_pointer(int_fcb));
                    util_print("BASICINT: ERROR: PRT exceeds runtime limit\r\n");
                    return 0;
                }
            }
        } else if (op == 28) {
            uint8_t length = 0, ch;
            if (code_size + 1 > INT_CODE_MAX) {
                util_call(16, util_pointer(int_fcb));
                util_print("BASICINT: ERROR: code exceeds runtime limit\r\n");
                return 0;
            }
            code[code_size++] = 0;
            while ((ch = next_int_byte()) != 0) {
                if (code_size == INT_CODE_MAX || length == 255) {
                    util_call(16, util_pointer(int_fcb));
                    util_print("BASICINT: ERROR: inline string exceeds runtime limit\r\n");
                    return 0;
                }
                code[code_size++] = ch;
                ++length;
            }
            code[code_size - length - 1] = length;
        } else if (op == 46 || op == 54 || op == 55 || op == 94 || op == 30) {
            uint8_t source_hi = next_int_byte(), source_lo = next_int_byte();
            uint16_t operand;
            if (code_size + 2 > INT_CODE_MAX) {
                util_call(16, util_pointer(int_fcb));
                util_print("BASICINT: ERROR: code exceeds runtime limit\r\n");
                return 0;
            }
            code[code_size++] = source_lo;
            code[code_size++] = source_hi;
            operand = (uint16_t)(source_lo | ((uint16_t)source_hi << 8));
            if (op == 46) {
                if (operand >= constant_count) {
                    util_call(16, util_pointer(int_fcb));
                    util_print("BASICINT: ERROR: invalid constant reference\r\n");
                    return 0;
                }
                code[code_size - 2] = (uint8_t)operand;
                code[code_size - 1] = (uint8_t)(operand >> 8);
            } else {
                if (operand) --operand;
                code[code_size - 2] = (uint8_t)operand;
                code[code_size - 1] = (uint8_t)(operand >> 8);
            }
        }
    }
    status = util_call(16, util_pointer(int_fcb));
    if (status == 0xff) {
        util_print("BASICINT: ERROR: error closing .INT file\r\n");
        return 0;
    }
    if (!code_size) {
        util_print("BASICINT: ERROR: empty .INT program\r\n");
        return 0;
    }
    if (code_size > declared_code || declared_code - code_size > 3 ||
        data_size > declared_data) {
        util_print("BASICINT: ERROR: inconsistent .INT size header\r\n");
        return 0;
    }
    return 1;
}

static void append_console(uint8_t c)
{
    if (print_pos >= INT_LINE) dump_console();
    print_buffer[print_pos++] = c;
}

static void dump_console(void)
{
    uint8_t end = print_pos;
    while (end && print_buffer[end - 1] == ' ') --end;
    if (end) {
        uint8_t i;
        for (i = 0; i < end; ++i) util_print_char(print_buffer[i]);
    }
    util_print_newline();
    print_pos = 0;
    while (print_pos < INT_LINE) print_buffer[print_pos++] = ' ';
    print_pos = 0;
}

static void print_number(float n)
{
    uint32_t whole, fraction, divisor;
    uint8_t digits[10], count = 0, i;
    if (n != n) { util_print("NaN"); return; }
    if (n < 0) { util_print_char('-'); n = -n; }
    if (n > 99999999.0f) { util_print("********"); return; }
    whole = (uint32_t)n;
    fraction = (uint32_t)((n - (float)whole) * 1000000.0f + 0.5f);
    if (fraction == 1000000UL) { ++whole; fraction = 0; }
    if (whole == 0) digits[count++] = 0;
    while (whole && count < 10) {
        digits[count++] = (uint8_t)(whole % 10);
        whole /= 10;
    }
    for (i = count; i; --i) util_print_char((uint8_t)('0' + digits[i - 1]));
    if (fraction) util_print_char('.');
    for (divisor = 100000UL; fraction && divisor; divisor /= 10) {
        util_print_char((uint8_t)('0' + fraction / divisor));
        fraction %= divisor;
    }
}

static void print_vm_number(float n)
{
    print_number(n);
    util_print_char(' ');
}

static void print_value_string(Value value)
{
    uint16_t i, n = string_length(value);
    for (i = 0; i < n; ++i) append_console(string_char(value, i));
}

static uint8_t get_console_line(void)
{
    uint8_t i;
    util_print_char('?');
    util_print_char(' ');
    input_buffer[0] = 127;
    input_buffer[1] = 0;
    util_set_dma(util_pointer(input_buffer));
    util_call(10, util_pointer(input_buffer));
    input_len = input_buffer[1];
    input_pos = 2;
    input_eol = 0;
    for (i = 0; i < input_len; ++i)
        if (input_buffer[2 + i] == 0x1a) {
            input_len = i;
            input_eol = 1;
            break;
        }
    return 1;
}

static uint8_t get_input_char(void)
{
    while (input_pos >= input_len + 2) {
        if (input_eol) {
            fail("console input aborted");
            return 0;
        }
        get_console_line();
        if (halted) return 0;
    }
    return input_buffer[input_pos++];
}

static void read_field(uint8_t want_string, uint8_t source)
{
    uint8_t quoted = 0, c = 0;
    uint16_t length = 0;
    if (source == 1 && input_pos >= input_len + 2)
        get_console_line();
    if (source == 2 && data_pos >= data_size) {
        fail("OUT OF DATA");
        field_result = zero_value();
        return;
    }
    if (source == 0 && active_file < file_count &&
        files[active_file].record_pos >= files[active_file].record_len)
        file_read_record(active_file);
    while (!halted) {
        if (source == 1) c = get_input_char();
        else if (source == 2) c = data[data_pos++];
        else c = file_get_char(active_file);
        if (c != ' ' && c != '\t') break;
    }
    if (c == '"') quoted = 1;
    else if (length < sizeof(field_buffer)) field_buffer[length++] = c;
    while (!halted) {
        if (source == 1) {
            if (input_pos >= input_len + 2) break;
            c = get_input_char();
        } else if (source == 2) {
            if (data_pos >= data_size) break;
            c = data[data_pos++];
        } else c = file_get_char(active_file);
        if ((quoted && c == '"') || (!quoted && (c == ',' || c == '\r' || c == '\n')))
            break;
        if (length < sizeof(field_buffer)) field_buffer[length++] = c;
    }
    if (quoted && source != 0) {
        while (!halted) {
            if (source == 1) {
                if (input_pos >= input_len + 2) break;
                c = get_input_char();
            } else if (source == 2) {
                if (data_pos >= data_size) break;
                c = data[data_pos++];
            } else c = file_get_char(active_file);
            if (c == ',' || c == '\r' || c == '\n') break;
        }
    }
    if (want_string)
        field_result = make_string(field_buffer, length);
    else {
        float result = 0, fraction = 0.1f;
        uint16_t i = 0;
        uint8_t negative = 0, decimal = 0;
        if (i < length && field_buffer[i] == '-') { negative = 1; ++i; }
        else if (i < length && field_buffer[i] == '+') ++i;
        for (; i < length; ++i) {
            if (field_buffer[i] == '.') { decimal = 1; continue; }
            if (field_buffer[i] < '0' || field_buffer[i] > '9') continue;
            if (decimal) { result += (field_buffer[i] - '0') * fraction; fraction *= .1f; }
            else result = result * 10.0f + (field_buffer[i] - '0');
        }
        field_result = number_value(negative ? -result : result);
    }
}

static void file_write_record(uint8_t slot)
{
    IntFile *file = &files[slot];
    if (!file->dirty) return;
    util_set_dma(util_pointer(file->record));
    if (util_call(21, util_pointer(file->fcb)) != 0) {
        fail("disk write error");
        return;
    }
    file->dirty = 0;
    file->record_pos = 0;
}

static void file_read_record(uint8_t slot)
{
    IntFile *file = &files[slot];
    uint8_t status;
    util_set_dma(util_pointer(file->record));
    status = util_call(20, util_pointer(file->fcb));
    if (status) {
        file->record_len = file->record_pos = 0;
        fail("end of file");
        return;
    }
    file->record_pos = 0;
    file->record_len = INT_RECORD;
}

static uint8_t file_get_char(uint8_t slot)
{
    IntFile *file;
    if (slot >= file_count || !files[slot].used) {
        fail("file is not open");
        return 0;
    }
    file = &files[slot];
    if (file->record_pos >= file->record_len) file_read_record(slot);
    if (halted) return 0;
    return file->record[file->record_pos++];
}

static void file_put_char(uint8_t slot, uint8_t value)
{
    IntFile *file;
    if (slot >= file_count || !files[slot].used) {
        fail("file is not open");
        return;
    }
    file = &files[slot];
    if (!file->write_mode) {
        (void)util_call(16, util_pointer(file->fcb));
        if (util_call(22, util_pointer(file->fcb)) == 0xff) {
            fail("cannot create output file");
            return;
        }
        file->write_mode = 1;
    }
    if (file->record_pos == INT_RECORD) file_write_record(slot);
    if (halted) return;
    file->record[file->record_pos++] = value;
    file->dirty = 1;
}

static uint8_t open_file(Value filename)
{
    uint8_t slot, text[256], i, pos = 0;
    uint16_t length;
    uint16_t fcb;
    uint8_t status;
    for (slot = 0; slot < INT_FILES; ++slot)
        if (!files[slot].used) break;
    if (slot == INT_FILES) {
        fail("too many open files");
        return 0;
    }
    copy_string(filename, text, &length);
    text[length] = 0;
    fcb = util_pointer(files[slot].fcb);
    for (i = 0; i < length; ++i)
        if (text[i] == ' ' || text[i] == '"') { text[i] = 0; break; }
    if (!util_parse_fcb(text, (uint8_t)i, &pos, fcb,
                        util_current_drive(), 0)) {
        fail("invalid filename");
        return 0;
    }
    status = util_call(15, fcb);
    if (status == 0xff) {
        util_clear_fcb(fcb);
        pos = 0;
        if (!util_parse_fcb(text, (uint8_t)i, &pos, fcb,
                            util_current_drive(), 0)) {
            fail("cannot open file");
            return 0;
        }
        status = util_call(22, fcb);
        if (status == 0xff) {
            fail("cannot open file");
            return 0;
        }
    }
    files[slot].used = 1;
    files[slot].record_pos = files[slot].record_len = 0;
    files[slot].write_mode = 0;
    files[slot].dirty = 0;
    if (status == 0xff) {
        uint8_t j;
        files[slot].write_mode = 1;
        for (j = 0; j < INT_RECORD; ++j) files[slot].record[j] = 0x1a;
    }
    if (slot >= file_count) file_count = slot + 1;
    active_file = slot;
    return slot;
}

static void close_file(uint8_t slot)
{
    if (slot >= file_count || !files[slot].used) return;
    file_write_record(slot);
    if (util_call(16, util_pointer(files[slot].fcb)) == 0xff)
        fail("disk close error");
    files[slot].used = 0;
    if (active_file == slot) active_file = 0;
}

static uint16_t branch_operand(void)
{
    uint16_t value;
    if (pc + 1 >= code_size) {
        fail("truncated branch");
        return 0;
    }
    value = (uint16_t)(code[pc] | ((uint16_t)code[pc + 1] << 8));
    pc += 2;
    return value;
}

static uint8_t is_zero(Value value)
{
    return value.kind == V_STRING || value.kind == V_ARRAY
        ? value.value == 0 : as_number(value) == 0;
}

static void set_result(float n)
{
    stack[sp] = number_value(n);
}

static void binary_numeric(uint8_t op)
{
    float left, right, result = 0;
    if (!sp) {
        fail("expression stack underflow");
        return;
    }
    left = as_number(stack[sp - 1]);
    right = as_number(stack[sp]);
    if (halted) return;
    switch (op) {
    case 0: result = left + right; break;
    case 1: result = left - right; break;
    case 2: result = left * right; break;
    case 3:
        if (right == 0) { fail("divide by zero"); return; }
        result = left / right;
        break;
    case 4:
        if (left == 0 && right == 0) result = 1;
        else if (left == 0) result = 0;
        else if (left < 0) { fail("negative base to fractional power"); return; }
        else result = powf(left, right);
        break;
    case 5: result = left < right; break;
    case 6: result = left > right; break;
    case 7: result = left == right; break;
    case 8: result = left != right; break;
    case 9: result = left >= right; break;
    case 10: result = left <= right; break;
    default: break;
    }
    stack[--sp] = number_value(result);
}

static void compare_strings(uint8_t op)
{
    Value a, b;
    uint16_t i, na, nb;
    int8_t comparison = 0;
    uint16_t n;
    if (!sp) {
        fail("expression stack underflow");
        return;
    }
    a = stack[sp];
    b = stack[sp - 1];
    na = string_length(a);
    nb = string_length(b);
    n = na < nb ? na : nb;
    for (i = 0; i < n; ++i) {
        uint8_t ca = string_char(b, i), cb = string_char(a, i);
        if (ca < cb) { comparison = -1; break; }
        if (ca > cb) { comparison = 1; break; }
    }
    if (!comparison && na != nb) comparison = na < nb ? -1 : 1;
    switch (op) {
    case 21: stack[--sp] = number_value(comparison < 0); break;
    case 22: stack[--sp] = number_value(comparison > 0); break;
    case 23: stack[--sp] = number_value(comparison == 0); break;
    case 24: stack[--sp] = number_value(comparison != 0); break;
    case 25: stack[--sp] = number_value(comparison >= 0); break;
    default: stack[--sp] = number_value(comparison <= 0); break;
    }
}

static uint16_t new_array(uint8_t rank, uint16_t *bounds)
{
    uint8_t slot, i;
    uint32_t count = 1;
    for (i = 0; i < rank; ++i) count *= (uint32_t)bounds[i] + 1;
    if (count + cell_count > INT_CELL_MAX || count > 65535u) {
        fail("array storage exhausted");
        return 0;
    }
    for (slot = 0; slot < 32 && arrays[slot].used; ++slot) { }
    if (slot == 32) {
        fail("too many arrays");
        return 0;
    }
    arrays[slot].used = 1;
    arrays[slot].rank = rank;
    arrays[slot].length = (uint16_t)count;
    arrays[slot].base = cell_count;
    for (i = 0; i < rank; ++i) arrays[slot].dimensions[i] = bounds[i];
    while (count--) cells[cell_count++] = zero_value();
    return slot;
}

static void read_console_input(uint8_t string_input)
{
    if (input_pos >= input_len + 2) get_console_line();
    read_field(string_input, 1);
    push(field_result);
}

static void write_record_value(Value value, uint8_t string_input)
{
    uint16_t i, length;
    uint8_t temp[256];
    if (files[active_file].dirty || files[active_file].record_pos) {
        if (!files[active_file].write_mode) files[active_file].write_mode = 1;
        file_put_char(active_file, ',');
    }
    if (string_input) {
        copy_string(value, temp, &length);
        file_put_char(active_file, '"');
        for (i = 0; i < length; ++i) file_put_char(active_file, temp[i]);
        file_put_char(active_file, '"');
    } else {
        float number = as_number(value), integral = (float)(int32_t)number;
        if (number < 0) { file_put_char(active_file, '-'); number = -number; integral = -integral; }
        {
            uint32_t n = (uint32_t)integral;
            uint8_t digits[10], count = 0;
            if (!n) digits[count++] = 0;
            while (n && count < 10) { digits[count++] = n % 10; n /= 10; }
            while (count) file_put_char(active_file, (uint8_t)('0' + digits[--count]));
        }
        if (number - integral > .0001f) {
            float fraction = number - integral;
            file_put_char(active_file, '.');
            for (i = 0; i < 6 && fraction > .0000004f; ++i) {
                fraction *= 10;
                {
                    uint8_t digit = (uint8_t)fraction;
                    file_put_char(active_file, (uint8_t)('0' + digit));
                    fraction -= digit;
                }
            }
        }
    }
}

static void execute(void)
{
    uint16_t steps = 0;
    pc = 0;
    sp = 1;
    stack[0] = zero_value();
    stack[1] = zero_value();
    data_pos = 0;
    string_used = 1;
    strings[0] = 0;
    while (pc < code_size && !halted) {
        uint8_t op = code[pc++];
        Value a, b, value;
        uint16_t operand;
        if (++steps == 0) steps = 1;
        if (op & 0x80) {
            uint16_t literal;
            if (pc >= code_size) { fail("truncated literal"); break; }
            literal = (uint16_t)(((uint16_t)(op & 0x3f) << 8) | code[pc++]);
            value = number_value((float)literal);
            push(value);
            if (op & 0x40) {
                uint16_t index = cell_index(value);
                stack[sp] = cells[index];
            }
            continue;
        }
        switch (op) {
        case 0: case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8: case 9: case 10:
            binary_numeric(op);
            break;
        case 11:
            if (!sp) { fail("NOT stack underflow"); break; }
            stack[sp] = number_value((float)~(int32_t)as_number(stack[sp]));
            break;
        case 12: case 13: case 93:
            if (!sp) { fail("logical stack underflow"); break; }
            a = stack[sp]; b = stack[sp - 1];
            if (op == 12) stack[--sp] = number_value((float)((int32_t)as_number(a) & (int32_t)as_number(b)));
            else if (op == 13) stack[--sp] = number_value((float)((int32_t)as_number(a) | (int32_t)as_number(b)));
            else stack[--sp] = number_value((float)((int32_t)as_number(a) ^ (int32_t)as_number(b)));
            break;
        case 14:
            if (!sp) { fail("LOD stack underflow"); break; }
            operand = cell_index(stack[sp]);
            stack[sp] = cells[operand];
            break;
        case 15:
            if (sp < 1) { fail("STO stack underflow"); break; }
            operand = cell_index(stack[sp - 1]);
            cells[operand] = stack[sp];
            stack[sp - 1] = stack[sp];
            --sp;
            break;
        case 16:
            return;
        case 17:
            (void)pop();
            break;
        case 18:
            if (sp) push(stack[sp]);
            else fail("DUP stack underflow");
            break;
        case 19:
            if (sp < 1) fail("XCH stack underflow");
            else { value = stack[sp]; stack[sp] = stack[sp - 1]; stack[sp - 1] = value; }
            break;
        case 20:
            if (sp < 1) { fail("STD stack underflow"); break; }
            operand = cell_index(stack[sp - 1]);
            cells[operand] = stack[sp];
            sp -= 2;
            break;
        case 21: case 22: case 23: case 24: case 25: case 26:
            compare_strings(op);
            break;
        case 27:
            if (sp < 1) { fail("STS stack underflow"); break; }
            operand = cell_index(stack[sp - 1]);
            cells[operand] = stack[sp];
            sp -= 2;
            break;
        case 28:
            if (pc >= code_size || pc + code[pc] + 1 > code_size) {
                fail("truncated inline string");
                break;
            }
            push(string_value(pc, S_CODE));
            pc += (uint16_t)code[pc] + 1;
            break;
        case 29:
            a = pop(); b = pop();
            {
                uint8_t text[256];
                uint16_t na, nb;
                if (string_length(a) + string_length(b) > 255) {
                    fail("concatenated string too long");
                    break;
                }
                copy_string(b, text, &na);
                copy_string(a, text + na, &nb);
                push(make_string(text, na + nb));
            }
            break;
        case 30:
            operand = branch_operand();
            if (return_depth == 16) {
                fail("GOSUB stack overflow");
                break;
            }
            returns[return_depth++] = pc;
            return_sp[return_depth - 1] = sp;
            pc = (uint16_t)(operand + 1);
            break;
        case 31:
            if (!return_depth) { fail("RETURN without GOSUB"); break; }
            --return_depth;
            operand = return_sp[return_depth];
            if (sp > operand) {
                value = stack[operand];
                sp = operand;
                stack[sp] = value;
            } else sp = operand;
            pc = returns[return_depth];
            break;
        case 32:
            if (pc >= code_size) { fail("truncated ROW"); break; }
            {
                uint8_t rank = code[pc++], i;
                uint16_t bounds[4], slot;
                if (!rank || rank > 4 || sp + 1 < rank) { fail("invalid array rank"); break; }
                for (i = 0; i < rank; ++i) {
                    bounds[rank - 1 - i] = as_integer(stack[sp - i]);
                    if (bounds[rank - 1 - i] > 255) fail("array bound too large");
                }
                sp -= rank;
                slot = new_array(rank, bounds);
                value = zero_value();
                value.kind = V_ARRAY;
                value.value = slot;
                push(value);
            }
            break;
        case 33:
            if (!sp) { fail("SUB stack underflow"); break; }
            a = pop();
            if (a.kind != V_ARRAY || a.value >= 32 || !arrays[a.value].used) {
                fail("invalid array");
                break;
            }
            {
                Array *array = &arrays[a.value];
                uint32_t offset = 0, multiplier = 1;
                uint8_t i;
                for (i = 0; i < array->rank; ++i) {
                    Value sub = pop();
                    uint16_t index = as_integer(sub);
                    if (index > array->dimensions[array->rank - 1 - i]) {
                        fail("subscript out of range");
                        break;
                    }
                    offset += (uint32_t)index * multiplier;
                    multiplier *= (uint32_t)array->dimensions[array->rank - 1 - i] + 1;
                }
                if (!halted) push(address_value(array->base + (uint16_t)offset));
            }
            break;
        case 34:
            read_console_input(0);
            break;
        case 35:
            if (!sp) { fail("WRV stack underflow"); break; }
            print_vm_number(as_number(stack[sp]));
            (void)pop();
            break;
        case 36:
            if (!sp) { fail("WST stack underflow"); break; }
            print_value_string(stack[sp]);
            (void)pop();
            break;
        case 37:
            fail("random-access file mode is not supported");
            break;
        case 38:
            if (!sp) { fail("RDB stack underflow"); break; }
            operand = as_integer(stack[sp]);
            if (!operand || operand > file_count || !files[operand - 1].used)
                fail("file is not open");
            else active_file = (uint8_t)(operand - 1);
            (void)pop();
            break;
        case 39:
            if (input_pos < input_len + 2) {
                if (input_pos >= input_len + 2 || input_buffer[input_pos] != '\r')
                    fail("too many input fields");
            }
            break;
        case 40:
            b = pop(); a = pop();
            if (as_integer(b) == 2) {
                util_call(2, as_integer(a));
            } else if (as_integer(b) == 1) {
                util_call(5, as_integer(a));
            } else fail("unsupported output port");
            break;
        case 41: read_field(0, 0); push(field_result); break;
        case 42: read_field(1, 0); push(field_result); break;
        case 43:
            if (!sp) { fail("WRN stack underflow"); break; }
            write_record_value(stack[sp], 0);
            (void)pop();
            break;
        case 44:
            if (!sp) { fail("WRS stack underflow"); break; }
            write_record_value(stack[sp], 1);
            (void)pop();
            break;
        case 45:
            if (!sp) { fail("OPN stack underflow"); break; }
            (void)open_file(stack[sp]);
            sp = sp ? sp - 1 : 0;
            break;
        case 46:
            operand = branch_operand();
            if (operand >= constant_count) {
                fail("invalid constant reference");
                break;
            }
            push(number_value(constants[operand]));
            break;
        case 47:
            data_pos = 0;
            break;
        case 48:
            if (!sp) fail("NEG stack underflow");
            else stack[sp] = number_value(-as_number(stack[sp]));
            break;
        case 49: read_console_input(1); break;
        case 50: case 51:
            break;
        case 52: dump_console(); break;
        case 53:
            if (print_zone < 3) {
                uint8_t target = (uint8_t)(14 + print_zone * 14);
                while (print_pos < target) append_console(' ');
                ++print_zone;
            } else dump_console();
            break;
        case 54:
            operand = branch_operand();
            pc = (uint16_t)(operand + 1);
            break;
        case 55:
            operand = branch_operand();
            if (!sp) { fail("BRC stack underflow"); break; }
            if (is_zero(stack[sp])) pc = (uint16_t)(operand + 1);
            (void)pop();
            break;
        case 56:
            if (!sp) { fail("BFC stack underflow"); break; }
            a = pop();
            if (!sp) { fail("BFC condition missing"); break; }
            if (is_zero(stack[sp])) {
                int16_t delta = (int16_t)as_integer(a);
                if ((int32_t)pc + delta < 0 || (int32_t)pc + delta >= code_size)
                    fail("branch outside code");
                else pc = (uint16_t)(pc + delta);
            }
            break;
        case 57:
            a = pop();
            {
                int16_t delta = (int16_t)as_integer(a);
                if ((int32_t)pc + delta < 0 || (int32_t)pc + delta >= code_size)
                    fail("branch outside code");
                else pc = (uint16_t)(pc + delta);
            }
            break;
        case 58:
            if (!sp) { fail("CVB stack underflow"); break; }
            stack[sp] = number_value((float)(int16_t)as_integer(stack[sp]));
            break;
        case 59:
            get_console_line();
            break;
        case 60: read_field(1, 2); push(field_result); break;
        case 61: read_field(0, 2); push(field_result); break;
        case 62:
            while (data_pos < data_size && data[data_pos++] != '\n') { }
            break;
        case 63:
            if (active_file >= file_count) fail("no open file");
            else {
                file_put_char(active_file, '\r');
                file_put_char(active_file, '\n');
                file_write_record(active_file);
            }
            break;
        case 64:
            if (!sp) { fail("CLS stack underflow"); break; }
            close_file((uint8_t)as_integer(stack[sp]));
            (void)pop();
            break;
        case 65:
            if (!sp) fail("ABS stack underflow");
            else stack[sp] = number_value(fabsf(as_number(stack[sp])));
            break;
        case 66:
            if (!sp) { fail("INT stack underflow"); break; }
            stack[sp] = number_value((float)(int32_t)as_number(stack[sp]));
            break;
        case 67:
            random_seed = (uint16_t)(random_seed * 25173u + 13849u);
            push(number_value((float)random_seed / 65536.0f));
            break;
        case 68:
            if (!sp) fail("SGN stack underflow");
            else {
                float n = as_number(stack[sp]);
                stack[sp] = number_value(n < 0 ? -1 : n > 0 ? 1 : 0);
            }
            break;
        case 69: if (sp) stack[sp] = number_value(sinf(as_number(stack[sp]))); else fail("SIN stack underflow"); break;
        case 70: if (sp) stack[sp] = number_value(cosf(as_number(stack[sp]))); else fail("COS stack underflow"); break;
        case 71: if (sp) stack[sp] = number_value(atanf(as_number(stack[sp]))); else fail("ATAN stack underflow"); break;
        case 72: if (sp) stack[sp] = number_value(tanf(as_number(stack[sp]))); else fail("TAN stack underflow"); break;
        case 73: if (sp) stack[sp] = number_value(sqrtf(as_number(stack[sp]))); else fail("SQR stack underflow"); break;
        case 74:
            if (!sp) { fail("TAB stack underflow"); break; }
            {
                uint16_t col = as_integer(stack[sp]);
                if (col > INT_LINE) col %= INT_LINE;
                while (print_pos < (col ? col - 1 : 0)) append_console(' ');
                print_zone = 0;
                (void)pop();
            }
            break;
        case 75: if (sp) stack[sp] = number_value(expf(as_number(stack[sp]))); else fail("EXP stack underflow"); break;
        case 76:
            push(number_value((float)(INT_CELL_MAX - cell_count)));
            break;
        case 77:
            random_seed = (uint16_t)(random_seed ^ (uint16_t)(pc * 40503u));
            break;
        case 78: if (sp && as_number(stack[sp]) > 0) stack[sp] = number_value(logf(as_number(stack[sp]))); else fail("LOG domain error"); break;
        case 79:
            push(number_value((float)(print_pos + 1)));
            break;
        case 80:
            if (!sp) { fail("INP stack underflow"); break; }
            fail("hardware input port is not supported");
            break;
        case 81:
            if (!sp) { fail("ASC stack underflow"); break; }
            a = pop();
            push(number_value(string_length(a) ? string_char(a, 0) : 0));
            break;
        case 82:
            if (!sp) { fail("CHR stack underflow"); break; }
            a = pop();
            {
                uint8_t c = (uint8_t)as_integer(a);
                push(make_string(&c, 1));
            }
            break;
        case 83: case 85: case 86:
            if (op == 85 && sp >= 2) {
                uint16_t start = as_integer(stack[sp - 1]);
                uint16_t n = as_integer(stack[sp]), i;
                uint8_t text[256];
                b = stack[sp - 2];
                if (!start) { fail("MID$ start must be positive"); break; }
                if (start > string_length(b)) n = 0;
                else if (n > string_length(b) - start + 1)
                    n = string_length(b) - start + 1;
                for (i = 0; i < n; ++i)
                    text[i] = string_char(b, start - 1 + i);
                sp -= 3;
                push(make_string(text, n));
            } else if (sp >= 1) {
                uint16_t count = as_integer(stack[sp]), n, i, start;
                b = stack[sp - 1];
                n = string_length(b);
                if (!count) { fail("string segment length must be positive"); break; }
                if (op == 83) start = 0;
                else start = count >= n ? 0 : n - count;
                if (op == 83 && count < n) n = count;
                else if (op == 86 && count < n) n = count;
                {
                    uint8_t text[256];
                    for (i = 0; i < n; ++i) text[i] = string_char(b, start + i);
                    sp -= 2;
                    push(make_string(text, n));
                }
            } else fail("string segment stack underflow");
            break;
        case 84:
            if (!sp) fail("LEN stack underflow");
            else stack[sp] = number_value((float)string_length(stack[sp]));
            break;
        case 87:
            if (!sp) { fail("STR stack underflow"); break; }
            {
                uint8_t text[32];
                float number = as_number(stack[sp]);
                uint8_t neg = number < 0, digits[12], n = 0, i;
                uint32_t whole = (uint32_t)(neg ? -number : number);
                if (neg) text[0] = '-';
                if (!whole) digits[n++] = 0;
                while (whole && n < 10) { digits[n++] = whole % 10; whole /= 10; }
                {
                    uint8_t pos = neg ? 1 : 0;
                    for (i = n; i; --i) text[pos++] = (uint8_t)('0' + digits[i - 1]);
                    n = pos;
                }
                (void)pop();
                push(make_string(text, n));
            }
            break;
        case 88:
            if (!sp) { fail("VAL stack underflow"); break; }
            a = pop();
            {
                uint8_t text[256], negative = 0, decimal = 0;
                uint16_t length, i = 0;
                float result = 0, fraction = .1f;
                copy_string(a, text, &length);
                if (i < length && text[i] == '-') { negative = 1; ++i; }
                for (; i < length; ++i) {
                    if (text[i] == '.') { decimal = 1; continue; }
                    if (text[i] >= '0' && text[i] <= '9') {
                        if (decimal) { result += (text[i] - '0') * fraction; fraction *= .1f; }
                        else result = result * 10 + text[i] - '0';
                    }
                }
                push(number_value(negative ? -result : result));
            }
            break;
        case 89: if (sp) stack[sp] = number_value(coshf(as_number(stack[sp]))); else fail("COSH stack underflow"); break;
        case 90: if (sp) stack[sp] = number_value(sinhf(as_number(stack[sp]))); else fail("SINH stack underflow"); break;
        case 91:
            if (sp) stack[sp] = number_value(floorf(as_number(stack[sp]) + .5f));
            else fail("RON stack underflow");
            break;
        case 92:
            if (sp < 1) fail("CKO stack underflow");
            else {
                a = pop(); b = pop();
                if (as_integer(a) > as_integer(b)) fail("ON statement index out of range");
                push(number_value(as_integer(a) * 3 + 1));
            }
            break;
        case 94:
            operand = branch_operand();
            if (sp) (void)pop();
            fail("file EOF branch is not supported");
            (void)operand;
            break;
        case 95:
            if (!sp) fail("BOL stack underflow");
            else { current_line = (uint8_t)as_integer(stack[sp]); (void)pop(); }
            break;
        case 96:
            if (!sp) fail("ADJ stack underflow");
            else stack[sp] = number_value(as_number(stack[sp]) + (float)pc);
            break;
        default:
            fail("unknown instruction");
            break;
        }
        if (sp >= INT_STACK_MAX) fail("expression stack overflow");
        if (pc > code_size) fail("program counter outside code");
    }
    if (!halted && pc >= code_size) {
        fail("program ended without XIT");
    }
}

void main(void)
{
    uint8_t i;
    current_line = 0;
    halted = 0;
    print_pos = print_zone = 0;
    file_count = active_file = 0;
    return_depth = 0;
    random_seed = 1;
    code_size = data_size = constant_count = 0;
    cell_count = 0;
    string_used = 1;
    strings[0] = 0;
    for (i = 0; i < INT_LINE; ++i) print_buffer[i] = ' ';
    for (i = 0; i < INT_FILES; ++i) files[i].used = 0;
    for (i = 0; i < 32; ++i) arrays[i].used = 0;
    for (i = 0; i < INT_CELL_MAX; ++i) cells[i] = zero_value();
    if (read_int_file()) {
        util_print_newline();
        execute();
        if (!halted && print_pos) dump_console();
        for (i = 0; i < file_count; ++i)
            if (files[i].used) close_file(i);
    }
}
