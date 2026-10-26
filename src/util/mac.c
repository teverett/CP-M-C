#include <stdint.h>
#include "util.h"
#include "../os/cpm.h"

#define MAX_SYMBOLS 256
#define NAME_SIZE 16
#define LINE_SIZE 128

typedef struct {
    char name[NAME_SIZE];
    uint16_t value;
    uint8_t defined;
} symbol;

static uint8_t args[129];
static uint8_t line[LINE_SIZE];
static uint8_t source_buffer[128];
static uint8_t output_buffer[128];
static symbol symbols[MAX_SYMBOLS];
static uint16_t input_fcb, output_fcb, dma, source_dma;
static uint16_t source_index, line_number, pc, record_address;
static uint8_t source_drive, output_drive, output_used, record_used;
static uint8_t record_data[16], failed, pass_number;

static void mac_flush_record(void);

static uint8_t upper(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
        return (uint8_t)(c - ('a' - 'A'));
    return c;
}

static void report(const char *message)
{
    util_print(message);
    util_print_newline();
}

static int symbol_index(const char *name, uint8_t create)
{
    uint16_t i;
    uint8_t length = 0;
    while (name[length] && length + 1 < NAME_SIZE)
        length++;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        uint8_t j = 0;
        if (!symbols[i].name[0])
            continue;
        while (j < length && symbols[i].name[j] == upper((uint8_t)name[j]))
            j++;
        if (j == length && !symbols[i].name[j])
            return (int)i;
    }
    if (!create)
        return -1;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (!symbols[i].name[0]) {
            uint8_t j;
            for (j = 0; j < length; j++)
                symbols[i].name[j] = (char)upper((uint8_t)name[j]);
            symbols[i].name[length] = 0;
            return (int)i;
        }
    }
    return -1;
}

static int source_char(void)
{
    uint8_t c;
    if (source_index >= 128) {
        uint8_t status;
        util_select(source_drive);
        util_set_dma(source_dma);
        status = util_call(20, input_fcb);
        if (status != 0)
            return -1;
        source_index = 0;
    }
    c = rb((uint16_t)(source_dma + source_index++));
    if (c == UTIL_EOF)
        return -1;
    return c;
}

static int next_line(void)
{
    uint8_t length = 0;
    int c;
    for (;;) {
        c = source_char();
        if (c < 0) {
            if (!length)
                return 0;
            break;
        }
        if (c == '\r')
            break;
        if (c == '\n') {
            if (length)
                break;
            continue;
        }
        if (length + 1 >= LINE_SIZE) {
            failed = 1;
            return -1;
        }
        line[length++] = (uint8_t)c;
    }
    {
        uint8_t quote = 0, i;
        for (i = 0; i < length; i++) {
            if (quote) {
                if (line[i] == quote)
                    quote = 0;
            } else if (line[i] == '\'' || line[i] == '"') {
                quote = line[i];
            } else if (line[i] == ';') {
                line[i] = 0;
                break;
            }
        }
    }
    line[length] = 0;
    line_number++;
    return 1;
}

static uint8_t open_source(void)
{
    util_select(source_drive);
    if (util_call(15, input_fcb) == 0xFF)
        return 0;
    wb((uint16_t)(input_fcb + 32), 0);
    source_index = 128;
    line_number = 0;
    return 1;
}

static uint8_t close_source(void)
{
    util_select(source_drive);
    return util_call(16, input_fcb) != 0xFF;
}

static void skip_space(uint8_t *pos)
{
    while (line[*pos] == ' ' || line[*pos] == '\t')
        (*pos)++;
}

static uint8_t identifier_char(uint8_t c)
{
    c = upper(c);
    return (uint8_t)((c >= 'A' && c <= 'Z') ||
                     (c >= '0' && c <= '9') || c == '_' || c == '.');
}

static uint8_t read_identifier(uint8_t *pos, char *out)
{
    uint8_t n = 0;
    while (identifier_char(line[*pos]) && n + 1 < NAME_SIZE)
        out[n++] = (char)upper(line[(*pos)++]);
    out[n] = 0;
    return n;
}

static int32_t expression(uint8_t *pos, int minimum, uint8_t *ok);

static int32_t atom(uint8_t *pos, uint8_t *ok)
{
    int32_t value = 0;
    uint8_t c;
    char name[NAME_SIZE];
    skip_space(pos);
    c = line[*pos];
    if (c == '+' || c == '-' || c == '~') {
        (*pos)++;
        value = atom(pos, ok);
        return c == '-' ? -value : c == '~' ? ~value : value;
    }
    if (c == '(') {
        (*pos)++;
        value = expression(pos, 1, ok);
        skip_space(pos);
        if (line[*pos] != ')')
            *ok = 0;
        else
            (*pos)++;
        return value;
    }
    if (c == '$') {
        (*pos)++;
        return pc;
    }
    if (c == '\'' || c == '"') {
        uint8_t quote = c;
        (*pos)++;
        if (!line[*pos]) {
            *ok = 0;
            return 0;
        }
        value = line[(*pos)++];
        if (line[*pos] == quote)
            (*pos)++;
        return value;
    }
    if (c >= '0' && c <= '9') {
        uint8_t start = *pos, end, base = 10;
        while (identifier_char(line[*pos]))
            (*pos)++;
        end = *pos;
        if (upper(line[end - 1]) == 'H') {
            base = 16;
            end--;
        } else if (upper(line[end - 1]) == 'B') {
            base = 2;
            end--;
        }
        while (start < end) {
            uint8_t d = upper(line[start++]);
            if (d >= '0' && d <= '9')
                d = (uint8_t)(d - '0');
            else if (d >= 'A' && d <= 'F')
                d = (uint8_t)(d - 'A' + 10);
            else {
                *ok = 0;
                return 0;
            }
            if (d >= base) {
                *ok = 0;
                return 0;
            }
            value = value * base + d;
        }
        return value;
    }
    if (identifier_char(c)) {
        int index;
        read_identifier(pos, name);
        index = symbol_index(name, 0);
        if (index < 0 || !symbols[index].defined) {
            if (pass_number == 2)
                *ok = 0;
            return 0;
        }
        return symbols[index].value;
    }
    *ok = 0;
    return 0;
}

static int precedence(uint8_t op)
{
    switch (op) {
    case '|': return 1;
    case '^': return 2;
    case '&': return 3;
    case '+': case '-': return 4;
    case '*': case '/': return 5;
    default: return 0;
    }
}

static int32_t expression(uint8_t *pos, int minimum, uint8_t *ok)
{
    int32_t left = atom(pos, ok);
    for (;;) {
        uint8_t op;
        int level;
        int32_t right;
        skip_space(pos);
        op = line[*pos];
        level = precedence(op);
        if (!level || level < minimum)
            break;
        (*pos)++;
        right = expression(pos, level + 1, ok);
        switch (op) {
        case '+': left += right; break;
        case '-': left -= right; break;
        case '*': left *= right; break;
        case '/':
            if (!right) *ok = 0;
            else left /= right;
            break;
        case '&': left &= right; break;
        case '|': left |= right; break;
        case '^': left ^= right; break;
        }
    }
    return left;
}

static uint8_t parse_expression(uint8_t *pos, int32_t *value)
{
    uint8_t ok = 1;
    *value = expression(pos, 1, &ok);
    skip_space(pos);
    if (line[*pos] && line[*pos] != ',' && line[*pos] != ';')
        ok = 0;
    return ok;
}

static uint8_t is_name(const char *a, const char *b)
{
    uint8_t i = 0;
    while (a[i] && b[i] && a[i] == b[i])
        i++;
    return (uint8_t)(!a[i] && !b[i]);
}

static uint8_t register_code(const char *name)
{
    if (name[1])
        return 0xFF;
    if (upper((uint8_t)name[0]) == 'B') return 0;
    if (upper((uint8_t)name[0]) == 'C') return 1;
    if (upper((uint8_t)name[0]) == 'D') return 2;
    if (upper((uint8_t)name[0]) == 'E') return 3;
    if (upper((uint8_t)name[0]) == 'H') return 4;
    if (upper((uint8_t)name[0]) == 'L') return 5;
    if (upper((uint8_t)name[0]) == 'M') return 6;
    if (upper((uint8_t)name[0]) == 'A') return 7;
    return 0xFF;
}

static uint8_t pair_code(const char *name)
{
    if (is_name(name, "B") || is_name(name, "BC")) return 0;
    if (is_name(name, "D") || is_name(name, "DE")) return 1;
    if (is_name(name, "H") || is_name(name, "HL")) return 2;
    if (is_name(name, "SP")) return 3;
    return 0xFF;
}

static uint8_t stack_pair_code(const char *name)
{
    if (is_name(name, "B") || is_name(name, "BC")) return 0;
    if (is_name(name, "D") || is_name(name, "DE")) return 1;
    if (is_name(name, "H") || is_name(name, "HL")) return 2;
    if (is_name(name, "PSW")) return 3;
    return 0xFF;
}

static void emit_byte(uint8_t value)
{
    if (pass_number == 2) {
        if (record_used && pc != (uint16_t)(record_address + record_used))
            mac_flush_record();
        if (!record_used)
            record_address = pc;
        record_data[record_used++] = value;
        if (record_used == sizeof record_data)
            mac_flush_record();
    }
    pc++;
}

static void emit_word(uint16_t value)
{
    emit_byte((uint8_t)value);
    emit_byte((uint8_t)(value >> 8));
}

static void output_char(uint8_t c)
{
    output_buffer[output_used++] = c;
    if (output_used == sizeof output_buffer) {
        util_select(output_drive);
        for (c = 0; c < sizeof output_buffer; c++)
            wb((uint16_t)(dma + c), output_buffer[c]);
        util_set_dma(dma);
        if (util_call(21, output_fcb) != 0)
            failed = 1;
        output_used = 0;
    }
}

static void output_flush(void)
{
    uint8_t i;
    if (!output_used || failed)
        return;
    for (i = output_used; i < sizeof output_buffer; i++)
        output_buffer[i] = 0x1A;
    util_select(output_drive);
    for (i = 0; i < sizeof output_buffer; i++)
        wb((uint16_t)(dma + i), output_buffer[i]);
    util_set_dma(dma);
    if (util_call(21, output_fcb) != 0)
        failed = 1;
    output_used = 0;
}

static uint8_t split_operands(uint8_t *pos, uint8_t *end)
{
    uint8_t depth = 0, quote = 0;
    while (line[*pos] == ' ' || line[*pos] == '\t')
        (*pos)++;
    *end = *pos;
    while (line[*end]) {
        uint8_t c = line[*end];
        if (quote) {
            if (c == quote)
                quote = 0;
        } else if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '(') {
            depth++;
        } else if (c == ')') {
            if (depth) depth--;
        } else if (c == ',' && depth == 0) {
            break;
        }
        (*end)++;
    }
    return (uint8_t)(*end > *pos);
}

static uint8_t alu_index(const char *name)
{
    if (is_name(name, "ADD")) return 0;
    if (is_name(name, "ADC")) return 1;
    if (is_name(name, "SUB")) return 2;
    if (is_name(name, "SBB")) return 3;
    if (is_name(name, "ANA")) return 4;
    if (is_name(name, "XRA")) return 5;
    if (is_name(name, "ORA")) return 6;
    if (is_name(name, "CMP")) return 7;
    return 0xFF;
}

static uint8_t alui_index(const char *name)
{
    if (is_name(name, "ADI")) return 0;
    if (is_name(name, "ACI")) return 1;
    if (is_name(name, "SUI")) return 2;
    if (is_name(name, "SBI")) return 3;
    if (is_name(name, "ANI")) return 4;
    if (is_name(name, "XRI")) return 5;
    if (is_name(name, "ORI")) return 6;
    if (is_name(name, "CPI")) return 7;
    return 0xFF;
}

static uint8_t jump_index(const char *name)
{
    if (is_name(name, "JNZ")) return 0;
    if (is_name(name, "JZ")) return 1;
    if (is_name(name, "JNC")) return 2;
    if (is_name(name, "JC")) return 3;
    if (is_name(name, "JPO")) return 4;
    if (is_name(name, "JPE")) return 5;
    if (is_name(name, "JP")) return 6;
    if (is_name(name, "JM")) return 7;
    return 0xFF;
}

static uint8_t call_index(const char *name)
{
    if (is_name(name, "CNZ")) return 0;
    if (is_name(name, "CZ")) return 1;
    if (is_name(name, "CNC")) return 2;
    if (is_name(name, "CC")) return 3;
    if (is_name(name, "CPO")) return 4;
    if (is_name(name, "CPE")) return 5;
    if (is_name(name, "CP")) return 6;
    if (is_name(name, "CM")) return 7;
    return 0xFF;
}

static uint8_t return_index(const char *name)
{
    if (is_name(name, "RNZ")) return 0;
    if (is_name(name, "RZ")) return 1;
    if (is_name(name, "RNC")) return 2;
    if (is_name(name, "RC")) return 3;
    if (is_name(name, "RPO")) return 4;
    if (is_name(name, "RPE")) return 5;
    if (is_name(name, "RP")) return 6;
    if (is_name(name, "RM")) return 7;
    return 0xFF;
}

static uint8_t assemble_instruction(const char *op, uint8_t *pos)
{
    char first[NAME_SIZE], second[NAME_SIZE];
    uint8_t a, b, end, r, p, ok, index;
    int32_t value;
    first[0] = second[0] = 0;
    if (is_name(op, "NOP")) { emit_byte(0x00); return 1; }
    if (is_name(op, "HLT")) { emit_byte(0x76); return 1; }
    if (is_name(op, "RET")) { emit_byte(0xC9); return 1; }
    if (is_name(op, "XCHG")) { emit_byte(0xEB); return 1; }
    if (is_name(op, "XTHL")) { emit_byte(0xE3); return 1; }
    if (is_name(op, "SPHL")) { emit_byte(0xF9); return 1; }
    if (is_name(op, "PCHL")) { emit_byte(0xE9); return 1; }
    if (is_name(op, "EI")) { emit_byte(0xFB); return 1; }
    if (is_name(op, "DI")) { emit_byte(0xF3); return 1; }
    if (is_name(op, "DAA")) { emit_byte(0x27); return 1; }
    if (is_name(op, "CMA")) { emit_byte(0x2F); return 1; }
    if (is_name(op, "STC")) { emit_byte(0x37); return 1; }
    if (is_name(op, "CMC")) { emit_byte(0x3F); return 1; }
    if (is_name(op, "RLC")) { emit_byte(0x07); return 1; }
    if (is_name(op, "RRC")) { emit_byte(0x0F); return 1; }
    if (is_name(op, "RAL")) { emit_byte(0x17); return 1; }
    if (is_name(op, "RAR")) { emit_byte(0x1F); return 1; }
    index = return_index(op);
    if (index != 0xFF) { emit_byte((uint8_t)(0xC0 + index * 8)); return 1; }
    if (is_name(op, "MOV") || is_name(op, "MVI") ||
        is_name(op, "LXI") || is_name(op, "INR") || is_name(op, "DCR") ||
        is_name(op, "PUSH") || is_name(op, "POP") || is_name(op, "INX") ||
        is_name(op, "DCX") || is_name(op, "DAD") || is_name(op, "LDAX") ||
        is_name(op, "STAX") || is_name(op, "IN") || is_name(op, "OUT")) {
        if (!split_operands(pos, &end)) return 0;
        if (is_name(op, "IN") || is_name(op, "OUT")) {
            uint8_t save = line[end];
            line[end] = 0;
            value = 0;
            ok = (uint8_t)parse_expression(pos, &value);
            line[end] = save;
            if (!ok || value < 0 || value > 255) return 0;
            emit_byte(is_name(op, "IN") ? 0xDB : 0xD3);
            emit_byte((uint8_t)value);
            return 1;
        }
        {
            uint8_t save = line[end];
            line[end] = 0;
            read_identifier(pos, first);
            line[end] = save;
        }
        if (is_name(op, "INR") || is_name(op, "DCR")) {
            r = register_code(first);
            if (r == 0xFF) return 0;
            emit_byte((uint8_t)((is_name(op, "INR") ? 0x04 : 0x05) + r * 8));
            return 1;
        }
        if (is_name(op, "PUSH") || is_name(op, "POP")) {
            p = stack_pair_code(first);
            if (p == 0xFF) return 0;
            emit_byte((uint8_t)((is_name(op, "PUSH") ? 0xC5 : 0xC1) + p * 16));
            return 1;
        }
        if (is_name(op, "INX") || is_name(op, "DCX") || is_name(op, "DAD") ||
            is_name(op, "LDAX") || is_name(op, "STAX")) {
            p = pair_code(first);
            if (p == 0xFF || (p == 3 && !is_name(op, "INX") &&
                             !is_name(op, "DCX") && !is_name(op, "DAD")))
                return 0;
            if (is_name(op, "LDAX") || is_name(op, "STAX")) {
                if (p > 1) return 0;
                emit_byte((uint8_t)((is_name(op, "LDAX") ? 0x0A : 0x02) + p * 16));
            } else {
                emit_byte((uint8_t)((is_name(op, "INX") ? 0x03 :
                                     is_name(op, "DCX") ? 0x0B : 0x09) + p * 16));
            }
            return 1;
        }
        if (end >= sizeof line || line[end] != ',') return 0;
        *pos = (uint8_t)(end + 1);
        if (!split_operands(pos, &end)) return 0;
        {
            uint8_t save = line[end];
            line[end] = 0;
            if (is_name(op, "MOV")) {
                read_identifier(pos, second);
                a = register_code(first);
                b = register_code(second);
                line[end] = save;
                if (a == 0xFF || b == 0xFF) return 0;
                emit_byte((uint8_t)(0x40 | (a << 3) | b));
                return 1;
            }
            value = 0;
            ok = (uint8_t)parse_expression(pos, &value);
            line[end] = save;
        }
        if (!ok) return 0;
        if (is_name(op, "MVI")) {
            r = register_code(first);
            if (r == 0xFF || value < 0 || value > 255) return 0;
            emit_byte((uint8_t)(0x06 + r * 8));
            emit_byte((uint8_t)value);
            return 1;
        }
        p = pair_code(first);
        if (p == 0xFF || value < 0 || value > 65535) return 0;
        emit_byte((uint8_t)(0x01 + p * 16));
        emit_word((uint16_t)value);
        return 1;
    }
    if (is_name(op, "JMP") || is_name(op, "CALL") ||
        is_name(op, "LDA") || is_name(op, "STA") ||
        is_name(op, "LHLD") || is_name(op, "SHLD") ||
        is_name(op, "RST")) {
        value = 0;
        if (!parse_expression(pos, &value)) return 0;
        if (is_name(op, "RST")) {
            if (value < 0 || value > 7) return 0;
            emit_byte((uint8_t)(0xC7 + value * 8));
            return 1;
        }
        if (value < 0 || value > 65535) return 0;
        emit_byte(is_name(op, "JMP") ? 0xC3 :
                  is_name(op, "CALL") ? 0xCD :
                  is_name(op, "LDA") ? 0x3A :
                  is_name(op, "STA") ? 0x32 :
                  is_name(op, "LHLD") ? 0x2A : 0x22);
        emit_word((uint16_t)value);
        return 1;
    }
    index = jump_index(op);
    if (index != 0xFF) {
        value = 0;
        if (!parse_expression(pos, &value) || value < 0 || value > 65535)
            return 0;
        emit_byte((uint8_t)(0xC2 + index * 8));
        emit_word((uint16_t)value);
        return 1;
    }
    index = call_index(op);
    if (index != 0xFF) {
        value = 0;
        if (!parse_expression(pos, &value) || value < 0 || value > 65535)
            return 0;
        emit_byte((uint8_t)(0xC4 + index * 8));
        emit_word((uint16_t)value);
        return 1;
    }
    index = alu_index(op);
    if (index != 0xFF) {
        if (!split_operands(pos, &end)) return 0;
        {
            uint8_t save = line[end];
            line[end] = 0;
            read_identifier(pos, first);
            line[end] = save;
        }
        r = register_code(first);
        if (r == 0xFF) return 0;
        emit_byte((uint8_t)(0x80 + index * 8 + r));
        return 1;
    }
    index = alui_index(op);
    if (index != 0xFF) {
        value = 0;
        if (!parse_expression(pos, &value) || value < 0 || value > 255)
            return 0;
        emit_byte((uint8_t)(0xC6 + index * 8));
        emit_byte((uint8_t)value);
        return 1;
    }
    if (is_name(op, "RST")) return 0;
    return 0;
}

static uint8_t operation_name(const char *name)
{
    return is_name(name, "ORG") || is_name(name, "DB") ||
        is_name(name, "DEFB") || is_name(name, "BYTE") ||
        is_name(name, "DW") || is_name(name, "DEFW") ||
        is_name(name, "DS") || is_name(name, "END") ||
        is_name(name, "TITLE") || is_name(name, "NAME") ||
        is_name(name, "PAGE") || is_name(name, "ASEG") ||
        is_name(name, "CSEG") || is_name(name, "DSEG") ||
        is_name(name, ".CPU") || is_name(name, "EQU") ||
        is_name(name, "SET") || is_name(name, "MOV") ||
        is_name(name, "MVI") || is_name(name, "LXI") ||
        is_name(name, "JMP") || is_name(name, "CALL") ||
        is_name(name, "RET") || is_name(name, "NOP") ||
        is_name(name, "HLT") || is_name(name, "XCHG") ||
        is_name(name, "XTHL") || is_name(name, "SPHL") ||
        is_name(name, "PCHL") || is_name(name, "EI") ||
        is_name(name, "DI") || is_name(name, "DAA") ||
        is_name(name, "CMA") || is_name(name, "STC") ||
        is_name(name, "CMC") || is_name(name, "RLC") ||
        is_name(name, "RRC") || is_name(name, "RAL") ||
        is_name(name, "RAR") || is_name(name, "INR") ||
        is_name(name, "DCR") || is_name(name, "PUSH") ||
        is_name(name, "POP") || is_name(name, "INX") ||
        is_name(name, "DCX") || is_name(name, "DAD") ||
        is_name(name, "LDAX") || is_name(name, "STAX") ||
        is_name(name, "IN") || is_name(name, "OUT") ||
        is_name(name, "LDA") || is_name(name, "STA") ||
        is_name(name, "LHLD") || is_name(name, "SHLD") ||
        is_name(name, "RST") || is_name(name, "ADD") ||
        is_name(name, "ADC") || is_name(name, "SUB") ||
        is_name(name, "SBB") || is_name(name, "ANA") ||
        is_name(name, "XRA") || is_name(name, "ORA") ||
        is_name(name, "CMP") || is_name(name, "ADI") ||
        is_name(name, "ACI") || is_name(name, "SUI") ||
        is_name(name, "SBI") || is_name(name, "ANI") ||
        is_name(name, "XRI") || is_name(name, "ORI") ||
        is_name(name, "CPI") || is_name(name, "JNZ") ||
        is_name(name, "JZ") || is_name(name, "JNC") ||
        is_name(name, "JC") || is_name(name, "JPO") ||
        is_name(name, "JPE") || is_name(name, "JP") ||
        is_name(name, "JM") || is_name(name, "CNZ") ||
        is_name(name, "CZ") || is_name(name, "CNC") ||
        is_name(name, "CC") || is_name(name, "CPO") ||
        is_name(name, "CPE") || is_name(name, "CP") ||
        is_name(name, "CM") || is_name(name, "RNZ") ||
        is_name(name, "RZ") || is_name(name, "RNC") ||
        is_name(name, "RC") || is_name(name, "RPO") ||
        is_name(name, "RPE") || is_name(name, "RP") ||
        is_name(name, "RM");
}

static void mac_flush_record(void)
{
    uint8_t i, sum;
    char hex[] = "0123456789ABCDEF";
    if (!record_used || failed)
        return;
    sum = (uint8_t)(record_used + (record_address >> 8) +
                    (record_address & 0xFF));
#define HEXBYTE(v) do { \
    uint8_t _b = (uint8_t)(v); \
    output_char((uint8_t)hex[_b >> 4]); \
    output_char((uint8_t)hex[_b & 15]); \
} while (0)
    output_char(':');
    HEXBYTE(record_used);
    HEXBYTE(record_address >> 8);
    HEXBYTE(record_address);
    output_char('0');
    output_char('0');
    for (i = 0; i < record_used; i++) {
        sum = (uint8_t)(sum + record_data[i]);
        HEXBYTE(record_data[i]);
    }
    HEXBYTE((uint8_t)(0 - sum));
    output_char('\r');
    output_char('\n');
#undef HEXBYTE
    record_used = 0;
}

static void write_eof_record(void)
{
    uint8_t i;
    char eof[] = ":00000001FF\r\n";
    if (failed) return;
    for (i = 0; eof[i]; i++)
        output_char((uint8_t)eof[i]);
    output_flush();
}

static uint8_t directive(const char *op, uint8_t *pos)
{
    uint8_t end, ok;
    int32_t value;
    if (is_name(op, "ORG")) {
        if (!parse_expression(pos, &value) || value < 0 || value > 65535)
            return 0;
        if (pass_number == 2 && record_used)
            mac_flush_record();
        pc = (uint16_t)value;
        return 1;
    }
    if (is_name(op, "DB") || is_name(op, "DEFB") ||
        is_name(op, "BYTE") || is_name(op, "DW") ||
        is_name(op, "DEFW") || is_name(op, "DS")) {
        if (is_name(op, "DS")) {
            if (!parse_expression(pos, &value) || value < 0 || value > 65535)
                return 0;
            while (value-- > 0)
                emit_byte(0);
            return 1;
        }
        while (line[*pos]) {
            skip_space(pos);
            if (line[*pos] == '\'' || line[*pos] == '"') {
                uint8_t quote = line[(*pos)++], closed = 0;
                while (line[*pos]) {
                    uint8_t c = line[(*pos)++];
                    if (c == quote) {
                        closed = 1;
                        break;
                    }
                    emit_byte(c);
                }
                if (!closed) return 0;
            } else {
                if (!split_operands(pos, &end)) return 0;
                {
                    uint8_t save = line[end];
                    ok = 1;
                    line[end] = 0;
                    value = expression(pos, 1, &ok);
                    skip_space(pos);
                    if (line[*pos]) ok = 0;
                    line[end] = save;
                }
                if (!ok) return 0;
                if (is_name(op, "DW") || is_name(op, "DEFW"))
                    emit_word((uint16_t)value);
                else
                    emit_byte((uint8_t)value);
            }
            skip_space(pos);
            if (!line[*pos]) break;
            if (line[*pos] != ',') return 0;
            (*pos)++;
        }
        return 1;
    }
    if (is_name(op, "END") || is_name(op, "TITLE") ||
        is_name(op, "NAME") || is_name(op, "PAGE") ||
        is_name(op, "ASEG") || is_name(op, "CSEG") ||
        is_name(op, "DSEG") || is_name(op, ".CPU"))
        return 1;
    return 0;
}

static uint8_t process_line(void)
{
    uint8_t pos = 0, first_start, first_end, op_end, colon = 0;
    char label[NAME_SIZE], op[NAME_SIZE];
    int index;
    label[0] = 0;
    op[0] = 0;
    while (line[pos] == ' ' || line[pos] == '\t')
        pos++;
    if (!line[pos] || line[pos] == ';')
        return 1;
    first_start = pos;
    first_end = pos;
    while (identifier_char(line[first_end]))
        first_end++;
    if (line[first_end] == ':') {
        colon = 1;
        line[first_end] = 0;
        pos = first_start;
        read_identifier(&pos, label);
        pos = (uint8_t)(first_end + 1);
        skip_space(&pos);
    } else {
        uint8_t save = line[first_end];
        line[first_end] = 0;
        pos = first_start;
        read_identifier(&pos, op);
        line[first_end] = save;
        if (first_end == first_start)
            return 1;
        if (operation_name(op)) {
            pos = first_end;
            skip_space(&pos);
        } else {
            uint8_t second_start, second_end;
            uint8_t saved = line[first_end];
            line[first_end] = 0;
            pos = first_start;
            read_identifier(&pos, label);
            line[first_end] = saved;
            pos = first_end;
            skip_space(&pos);
            second_start = pos;
            second_end = pos;
            while (identifier_char(line[second_end]))
                second_end++;
            if (second_end == second_start)
                return 1;
            saved = line[second_end];
            line[second_end] = 0;
            read_identifier(&pos, op);
            line[second_end] = saved;
            pos = second_end;
            skip_space(&pos);
            colon = 2;
        }
    }
    if (colon == 1) {
        op_end = pos;
        read_identifier(&op_end, op);
        pos = op_end;
    }
    if (!op[0])
        return 1;
    if (is_name(op, "EQU") || is_name(op, "SET")) {
        uint8_t ok = 1;
        int32_t value = expression(&pos, 1, &ok);
        if (!ok && pass_number == 2) return 0;
        index = symbol_index(label, 1);
        if (index < 0) return 0;
        if (pass_number == 1) {
            symbols[index].value = (uint16_t)value;
            symbols[index].defined = 1;
        }
        return 1;
    }
    if (label[0]) {
        index = symbol_index(label, 1);
        if (index < 0) return 0;
        if (pass_number == 1) {
            symbols[index].value = pc;
            symbols[index].defined = 1;
        }
    }
    if (directive(op, &pos))
        return 1;
    return assemble_instruction(op, &pos);
}

static uint8_t run_pass(uint8_t pass)
{
    int status;
    pass_number = pass;
    if (pass == 2)
        output_used = 0;
    pc = 0x0100;
    record_used = 0;
    if (!open_source())
        return 0;
    while ((status = next_line()) > 0) {
        if (!process_line()) {
            failed = 1;
            util_print("MAC ERROR AT LINE ");
            util_print_decimal(line_number);
            util_print_newline();
            break;
        }
    }
    if (status < 0)
        failed = 1;
    if (!close_source())
        failed = 1;
    if (pass == 2 && !failed) {
        mac_flush_record();
        write_eof_record();
    }
    return (uint8_t)!failed;
}

void main(void)
{
    uint8_t length = util_args(args), pos = 0, i;
    uint16_t symbol_number;
    uint8_t current_drive;
    current_drive = util_current_drive();
    input_fcb = util_fcb_address(0);
    output_fcb = util_fcb_address(1);
    dma = util_dma_buffer();
    source_dma = util_pointer(source_buffer);
    while (pos < length && args[pos] == ' ')
        pos++;
    if (!util_parse_fcb(args, length, &pos, input_fcb,
                        (uint8_t)(current_drive + 1), 0)) {
        report("MAC SOURCE.ASM");
        return;
    }
    if (pos < length && args[pos] != ' ') {
        report("INVALID PARAMETER");
        return;
    }
    if (rb((uint16_t)(input_fcb + 9)) == ' ') {
        wb((uint16_t)(input_fcb + 9), 'A');
        wb((uint16_t)(input_fcb + 10), 'S');
        wb((uint16_t)(input_fcb + 11), 'M');
    }
    source_drive = rb(input_fcb) ? (uint8_t)(rb(input_fcb) - 1) : current_drive;
    output_drive = source_drive;
    util_select(source_drive);
    if (!open_source()) {
        report("NO SOURCE FILE PRESENT");
        return;
    }
    close_source();
    util_clear_fcb(output_fcb);
    wb(output_fcb, (uint8_t)(source_drive + 1));
    for (i = 1; i < 9; i++)
        wb((uint16_t)(output_fcb + i), rb((uint16_t)(input_fcb + i)));
    wb((uint16_t)(output_fcb + 9), 'H');
    wb((uint16_t)(output_fcb + 10), 'E');
    wb((uint16_t)(output_fcb + 11), 'X');
    util_select(output_drive);
    (void)util_call(19, output_fcb);
    if (util_call(22, output_fcb) == 0xFF) {
        report("NO DIRECTORY SPACE");
        return;
    }
    for (symbol_number = 0; symbol_number < MAX_SYMBOLS; symbol_number++) {
        symbols[symbol_number].name[0] = 0;
        symbols[symbol_number].defined = 0;
    }
    failed = 0;
    if (!run_pass(1) || !run_pass(2)) {
        (void)util_call(19, output_fcb);
        (void)util_call(16, output_fcb);
        (void)util_call(19, output_fcb);
        report("ASSEMBLY FAILED");
        return;
    }
    util_select(output_drive);
    if (util_call(16, output_fcb) == 0xFF) {
        report("CANNOT CLOSE OUTPUT");
        return;
    }
    util_select(current_drive);
    report("ASSEMBLY COMPLETE");
}
