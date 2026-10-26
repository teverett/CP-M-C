/*
 * Direct-source BASIC-E subset for CP/M.  This is not the BASIC-E .INT VM.
 * All storage is bounded and static; expression recursion uses small frames.
 * Limits: 6142 source bytes, 256 indexed statements, 32 identifiers (23
 * characters), 192 scalar/array cells, rank 3, 512-byte string heap and
 * scratch arena, 255 bytes per string, 16 FOR/GOSUB levels, 8 expression levels.
 * Scalars start at zero/empty; arrays require executable DIM and use inclusive
 * bounds starting at zero. FOR is the BASIC-E post-tested, dynamic-bound form.
 * File I/O, DEF, FRE, hyperbolic/hardware functions and compiler directives are not
 * implemented. INPUT accepts comma-separated fields and fails on bad input
 * rather than retrying. Boolean results are 0/1, numbers are IEEE single,
 * printed with six significant digits. Strings cannot contain NUL. The PRNG
 * is deterministic; RANDOMIZE without a seed mixes source/PC/console status.
 * Link data after both _CODE and the SDCC float helpers in _HOME, not merely
 * after _CODE. No writable object requires _INITIALIZED startup copying.
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include "../os/cpm.h"
#include "../util/util.h"

#define SOURCE_SIZE 6144
#define STATEMENTS 256
#define SYMBOLS 32
#define CELLS 192
#define STRINGS 512
#define SCRATCH 512
#define VALUES 24
#define DEPTH 16
#define EXPRESSION_DEPTH 8
#define NAME_SIZE 24
#define NONE 65535u

typedef struct { uint16_t start, end, jump, label; } Statement;
typedef struct {
    char name[NAME_SIZE];
    uint16_t base, dim[3];
    uint8_t rank;
} Symbol;
typedef struct { float n; uint16_t s; } Cell;
typedef struct { float n; char *s; } Value;
typedef struct {
    uint16_t cell, body, bound, finish;
    uint8_t calls;
} Loop;

static char source[SOURCE_SIZE], strings[STRINGS], scratch[SCRATCH];
static Statement statements[STATEMENTS];
static Symbol symbols[SYMBOLS];
static Cell cells[CELLS];
static Value values[VALUES];
static Loop loops[DEPTH];
static uint16_t returns[DEPTH];
static uint16_t count, pc, nextpc, source_len, used_cells, string_used;
static uint16_t scratch_used, data_pc, data_pos, rng;
static uint8_t used_symbols, vp, recursion, loop_count, call_count, column;
static uint8_t pending_key;
static char *p, *end;
static uint8_t console_buffer[130], arguments[128];
static jmp_buf abort_run;

static uint8_t upper(uint8_t c)
{
    return c >= 'a' && c <= 'z' ? c - 32 : c;
}

static uint8_t letter(uint8_t c)
{
    c = upper(c);
    return c >= 'A' && c <= 'Z';
}

static uint8_t digit(uint8_t c) { return c >= '0' && c <= '9'; }
static uint8_t namechar(uint8_t c)
{
    return letter(c) || digit(c) || c == '.' || c == '$';
}

static void fail(const char *message)
{
    uint16_t i, line = 1;
    util_print("\r\nERROR: ");
    util_print(message);
    if (pc < count) {
        for (i = 0; i < statements[pc].start; ++i)
            if (source[i] == '\n') ++line;
        util_print(" at source line");
        util_print_decimal(line);
    }
    util_print_newline();
    longjmp(abort_run, 1);
}

static void spaces(void)
{
    while (p < end && (*p == ' ' || *p == '\t')) ++p;
}

static uint8_t word_at(const char *s, const char *word)
{
    while (*word)
        if (upper((uint8_t)*s++) != (uint8_t)*word++) return 0;
    return !namechar((uint8_t)*s);
}

static uint8_t keyword(const char *word)
{
    spaces();
    if (!word_at(p, word)) return 0;
    p += strlen(word);
    return 1;
}

static uint8_t take(char c)
{
    spaces();
    if (p < end && *p == c) { ++p; return 1; }
    return 0;
}

static void need(char c)
{
    if (!take(c)) fail("Expected punctuation");
}

static void finished(void)
{
    spaces();
    if (p < end) fail("Unexpected text");
}

static void identifier(char *name)
{
    uint8_t n = 0;
    spaces();
    if (!letter((uint8_t)*p)) fail("Expected identifier");
    while (p < end && namechar((uint8_t)*p)) {
            if (n == NAME_SIZE - 1) fail("Identifier exceeds 23 characters");
        name[n++] = upper((uint8_t)*p++);
    }
    name[n] = 0;
    /* A dollar sign is a type suffix, not an interior character. */
    if (strchr(name, '$') && name[n - 1] != '$')
        fail("Invalid $ suffix");
}

static uint8_t string_symbol(uint8_t s)
{
    char *name = symbols[s].name;
    return name[strlen(name) - 1] == '$';
}

static float random_number(void)
{
    rng = rng * 25173u + 13849u;
    return (float)rng / 65536.0f;
}

static uint8_t symbol(const char *name)
{
    uint8_t i;
    for (i = 0; i < used_symbols; ++i)
        if (!strcmp(symbols[i].name, name)) return i;
    if (used_symbols == SYMBOLS) fail("Variable limit (32)");
    if (used_cells == CELLS) fail("Variable storage full");
    i = used_symbols++;
    strcpy(symbols[i].name, name);
    symbols[i].base = used_cells++;
    symbols[i].rank = 0;
    return i;
}

static char *temporary(uint16_t length)
{
    char *s;
    if (length > 255 || scratch_used + length + 1 > SCRATCH)
        fail("String expression too large");
    s = scratch + scratch_used;
    scratch_used += length + 1;
    s[length] = 0;
    return s;
}

static void push_number(float n)
{
    if (vp == VALUES) fail("Expression too complex");
    values[vp].n = n;
    values[vp++].s = 0;
}

static void push_string(char *s)
{
    if (vp == VALUES) fail("Expression too complex");
    values[vp++].s = s;
}

static float number(uint8_t i)
{
    if (values[i].s) fail("Numeric value required");
    return values[i].n;
}

static char *text(uint8_t i)
{
    if (!values[i].s) fail("String value required");
    return values[i].s;
}

static float finite_number(float n)
{
    if (n != n || n > 3.4028234e38f || n < -3.4028234e38f)
        fail("Numeric overflow/domain");
    return n;
}

static int16_t integer(float n)
{
    if (n < -32767.0f || n > 32767.0f)
        fail("Integer out of range");
    return (int16_t)n;
}

/* No printf float conversion library is pulled into the transient. */
static char *format_number(float n)
{
    char *s = temporary(24), *q = s;
    int16_t exponent = 0;
    uint8_t i, d;
    finite_number(n);
    if (n < 0) { *q++ = '-'; n = -n; }
    if (n == 0) { *q++ = '0'; *q = 0; return s; }
    while (n >= 10) { n /= 10; ++exponent; }
    while (n < 1) { n *= 10; --exponent; }
    n += 0.000005f;
    if (n >= 10) { n /= 10; ++exponent; }
    /* Six significant digits; fixed notation for the everyday range. */
    if (exponent >= -4 && exponent < 6) {
        if (exponent < 0) {
            *q++ = '0'; *q++ = '.';
            for (i = 1; i < -exponent; ++i) *q++ = '0';
        }
        for (i = 0; i < 6; ++i) {
            d = (uint8_t)n; *q++ = '0' + d; n = (n - d) * 10;
            if (exponent >= 0 && i == exponent && i != 5) *q++ = '.';
        }
        if (strchr(s, '.')) {
            while (q[-1] == '0') --q;
            if (q[-1] == '.') --q;
        }
    } else {
        for (i = 0; i < 6; ++i) {
            d = (uint8_t)n; *q++ = '0' + d; n = (n - d) * 10;
            if (!i) *q++ = '.';
        }
        while (q[-1] == '0') --q;
        if (q[-1] == '.') --q;
        *q++ = 'E'; *q++ = exponent < 0 ? '-' : '+';
        if (exponent < 0) exponent = -exponent;
        *q++ = '0' + exponent / 10; *q++ = '0' + exponent % 10;
    }
    *q = 0;
    return s;
}

static float literal(void)
{
    float n = 0, scale = 1;
    int16_t exponent = 0;
    uint8_t got = 0, neg = 0;
    spaces();
    while (p < end && digit(*p)) { n = n * 10 + *p++ - '0'; got = 1; }
    if (p < end && *p == '.') {
        ++p;
        while (p < end && digit(*p)) {
            scale *= .1f; n += (*p++ - '0') * scale; got = 1;
        }
    }
    if (!got) fail("Expected number");
    if (p < end && upper(*p) == 'E') {
        ++p;
        if (*p == '+' || *p == '-') neg = *p++ == '-';
        if (!digit(*p)) fail("Invalid exponent");
        while (p < end && digit(*p)) {
            exponent = exponent * 10 + *p++ - '0';
            if (exponent > 38) fail("Exponent out of range");
        }
        while (exponent--) n = neg ? n / 10 : n * 10;
    }
    return finite_number(n);
}

static void expression(uint8_t minimum);

static uint16_t variable(uint8_t *which)
{
    char name[NAME_SIZE];
    uint8_t s, d = 0;
    uint16_t offset = 0, sub;
    identifier(name);
    s = symbol(name);
    if (take('(')) {
        if (!symbols[s].rank) fail("Array not dimensioned");
        do {
            if (d == symbols[s].rank) fail("Too many subscripts");
            expression(1);
            sub = integer(floorf(number(--vp) + .5f));
            if (sub >= symbols[s].dim[d]) fail("Subscript out of bounds");
            offset = offset * symbols[s].dim[d++] + sub;
        } while (take(','));
        need(')');
        if (d != symbols[s].rank) fail("Wrong subscript count");
    } else if (symbols[s].rank) fail("Array needs subscripts");
    *which = s;
    return symbols[s].base + offset;
}

static void store(uint16_t cell, uint8_t is_string, uint8_t v)
{
    uint16_t old, length, i, position;
    char *s;
    if (!is_string) { cells[cell].n = number(v); return; }
    s = text(v);
    length = strlen(s);
    old = cells[cell].s;
    /* Copy first: RHS may refer to the old value or another heap string. */
    if (s >= strings && s < strings + STRINGS) {
        char *copy = temporary(length);
        strcpy(copy, s); s = copy;
    }
    if (old) {
        position = old - 1;
        old = strlen(strings + position) + 1;
        memmove(strings + position, strings + position + old,
                string_used - position - old);
        string_used -= old;
        for (i = 0; i < used_cells; ++i)
            if (cells[i].s > position) cells[i].s -= old;
        cells[cell].s = 0;
    }
    if (!length) return;
    if (string_used + length + 1 > STRINGS) fail("String storage full");
    cells[cell].s = string_used + 1;
    strcpy(strings + string_used, s);
    string_used += length + 1;
}

static const char *const function_names[] = {
    "ABS", "INT", "SGN", "SIN", "COS", "TAN", "ATN", "SQR", "EXP",
    "LOG", "RND", "LEN", "LEFT$", "MID$", "RIGHT$", "STR$", "VAL",
    "ASC", "CHR$", "TAB", "POS", 0
};

static void function(uint8_t f)
{
    uint8_t base = vp, argc;
    float n = 0;
    int16_t start = 0, length = 0;
    char *s = 0, *q;
    need('(');
    if (!take(')')) {
        do { expression(1); } while (take(','));
        need(')');
    }
    argc = vp - base;
    if (argc != 1 && !(f == 12 || f == 13 || f == 14))
        fail("Function arity");
    if (f == 12 || f == 14) {
        if (argc != 2) fail("Substring arity");
    }
    if (f == 13 && argc != 2 && argc != 3)
        fail("MID$ needs 2 or 3 arguments");
    if (f <= 10 || f == 15 || f == 18 || f == 19 || f == 20)
        n = number(base);
    else s = text(base);
    switch (f) {
    case 0: if (n < 0) n = -n; break;
    case 1: n = floorf(n); break;
    case 2: n = n > 0 ? 1 : n < 0 ? -1 : 0; break;
    case 3: n = sinf(n); break;
    case 4: n = cosf(n); break;
    case 5: n = tanf(n); break;
    case 6: n = atanf(n); break;
    case 7: if (n < 0) fail("SQR domain"); n = sqrtf(n); break;
    case 8: if (n > 88) fail("EXP overflow"); n = expf(n); break;
    case 9: if (n <= 0) fail("LOG domain"); n = logf(n); break;
    case 10:
        if (n < 0) rng = (uint16_t)integer(-n);
        n = n == 0 ? (float)rng / 65536.0f : random_number(); break;
    case 11: n = strlen(s); break;
    case 12: case 13: case 14:
        length = integer(number(base + 1));
        if (length < 0) fail("Negative substring argument");
        if (f == 13) {
            if (!length) fail("MID$ starts at 1");
            start = length - 1;
            length = argc == 3 ? integer(number(base + 2)) : strlen(s);
            if (length < 0) fail("Negative substring length");
        } else if (f == 14) {
            start = strlen(s) - length;
            if (start < 0) start = 0;
        }
        if (start > (int16_t)strlen(s)) start = strlen(s);
        if (length > (int16_t)strlen(s) - start) length = strlen(s) - start;
        q = temporary(length); memcpy(q, s + start, length); s = q;
        break;
    case 15: s = format_number(n); break;
    case 16: {
        char *save = p, *save_end = end;
        p = s; end = s + strlen(s);
        spaces();
        start = 1;
        if (*p == '+' || *p == '-') start = *p++ == '-' ? -1 : 1;
        n = literal() * start;
        finished(); p = save; end = save_end;
        break;
    }
    case 17: if (!*s) fail("ASC of empty string"); n = (uint8_t)*s; break;
    case 18:
        start = integer(n);
        if (start < 1 || start > 255) fail("CHR$ range is 1..255");
        s = temporary(1); s[0] = start; break;
    case 19:
        start = integer(n);
        if (start < 0 || start > 127) fail("TAB range is 0..127");
        length = start > column ? start - column : 0;
        s = temporary(length); memset(s, ' ', length); break;
    case 20: n = column + 1; break;
    }
    vp = base;
    if (f == 12 || f == 13 || f == 14 || f == 15 || f == 18 || f == 19)
        push_string(s);
    else push_number(finite_number(n));
}

static void primary(void)
{
    char name[NAME_SIZE], *save, *s;
    uint16_t cell, length;
    uint8_t i, sym;
    spaces();
    if (take('(')) { expression(1); need(')'); return; }
    if (take('"')) {
        save = p; length = 0;
        while (p < end) {
            if (*p++ == '"') {
                if (p < end && *p == '"') ++p;
                else break;
            }
            ++length;
        }
        if (p > end || p[-1] != '"') fail("Unterminated string");
        s = temporary(length);
        for (i = 0; length; --length) {
            s[i++] = *save++;
            if (save[-1] == '"' && *save == '"') ++save;
        }
        push_string(s); return;
    }
    if (p < end && (digit(*p) || *p == '.')) { push_number(literal()); return; }
    save = p; identifier(name);
    if (!strcmp(name, "FRE")) fail("FRE unsupported");
    spaces();
    if (*p != '(' && !strcmp(name, "RND")) {
        push_number(random_number()); return;
    }
    if (*p != '(' && !strcmp(name, "POS")) {
        push_number(column + 1); return;
    }
    for (i = 0; function_names[i]; ++i)
        if (!strcmp(name, function_names[i])) { function(i); return; }
    p = save; cell = variable(&sym);
    if (string_symbol(sym))
        push_string(cells[cell].s ? strings + cells[cell].s - 1 : "");
    else push_number(cells[cell].n);
}

/* Precedence climbing, with a shared bounded value stack instead of large
 * recursive Value objects. Power is right associative and outranks unary -. */
static uint8_t operator_token(uint8_t *op)
{
    spaces();
    if (keyword("OR")) { *op = 1; return 1; }
    if (keyword("AND")) { *op = 2; return 2; }
    if (keyword("EQ")) { *op = 3; return 3; }
    if (keyword("NE")) { *op = 4; return 3; }
    if (keyword("LT")) { *op = 5; return 3; }
    if (keyword("LE")) { *op = 6; return 3; }
    if (keyword("GT")) { *op = 7; return 3; }
    if (keyword("GE")) { *op = 8; return 3; }
    if (p >= end) return 0;
    switch (*p++) {
    case '=': *op = 3; return 3;
    case '<': *op = take('>') ? 4 : take('=') ? 6 : 5; return 3;
    case '>': *op = take('=') ? 8 : 7; return 3;
    case '+': *op = 9; return 4;
    case '-': *op = 10; return 4;
    case '*': *op = 11; return 5;
    case '/': *op = 12; return 5;
    case '^': *op = 13; return 7;
    default: --p; return 0;
    }
}

static void expression(uint8_t minimum)
{
    uint8_t op, precedence, left;
    char *save, *a, *b, *s;
    float x, y, power;
    int16_t comparison;
    uint16_t length, exponent;
    if (++recursion > EXPRESSION_DEPTH) fail("Expression nesting too deep");
    if (take('-')) { expression(6); values[vp - 1].n = -number(vp - 1); }
    else if (take('+')) { expression(6); (void)number(vp - 1); }
    else if (keyword("NOT")) {
        expression(3); values[vp - 1].n = number(vp - 1) == 0;
    } else primary();
    for (;;) {
        save = p;
        precedence = operator_token(&op);
        if (!precedence || precedence < minimum) { p = save; break; }
        left = vp - 1;
        expression(op == 13 ? precedence : precedence + 1);
        if (values[left].s || values[left + 1].s) {
            a = text(left); b = text(left + 1);
            if (op == 9) {
                length = strlen(a) + strlen(b);
                s = temporary(length); strcpy(s, a); strcat(s, b);
                values[left].s = s;
            } else {
                if (op < 3 || op > 8) fail("Invalid string operator");
                comparison = strcmp(a, b);
                x = op == 3 ? comparison == 0 : op == 4 ? comparison != 0 :
                    op == 5 ? comparison < 0 : op == 6 ? comparison <= 0 :
                    op == 7 ? comparison > 0 : comparison >= 0;
                values[left].s = 0; values[left].n = x;
            }
        } else {
            x = number(left); y = number(left + 1);
            switch (op) {
            case 1: x = x != 0 || y != 0; break;
            case 2: x = x != 0 && y != 0; break;
            case 3: x = x == y; break;
            case 4: x = x != y; break;
            case 5: x = x < y; break;
            case 6: x = x <= y; break;
            case 7: x = x > y; break;
            case 8: x = x >= y; break;
            case 9: x += y; break;
            case 10: x -= y; break;
            case 11: x *= y; break;
            case 12: if (!y) fail("Division by zero"); x /= y; break;
            case 13:
                if ((!x && y <= 0) || (x < 0 && floorf(y) != y))
                    fail("Power domain");
                if (y >= -32767 && y <= 32767 && floorf(y) == y) {
                    exponent = (uint16_t)(y < 0 ? -y : y);
                    power = 1;
                    while (exponent) {
                        if (exponent & 1) power *= x;
                        exponent >>= 1;
                        if (exponent) x = finite_number(x * x);
                    }
                    if (!power && y < 0) fail("Power overflow");
                    x = y < 0 ? 1 / power : power;
                } else x = powf(x, y);
                break;
            }
            values[left].n = finite_number(x);
        }
        vp = left + 1;
    }
    --recursion;
}

static float numeric_expression(void)
{
    expression(1);
    return number(--vp);
}

static uint16_t target(float n)
{
    uint16_t i, label;
    if (n < 1 || n > 65535.0f || floorf(n) != n)
        fail("Invalid line label");
    label = (uint16_t)n;
    for (i = 0; i < count; ++i)
        if (statements[i].label == label) return i;
    fail("Undefined line label");
    return 0;
}

static void output(char *s)
{
    while (*s) {
        util_print_char(*s++);
        if (column != 255) ++column;
    }
}

static void newline(void) { util_print_newline(); column = 0; }

static void print_statement(void)
{
    uint8_t separator = 0;
    spaces();
    if (p < end && *p == '#') fail("File I/O not supported");
    while (p < end) {
        expression(1);
        output(values[vp - 1].s ? values[vp - 1].s :
               format_number(values[vp - 1].n));
        --vp;
        separator = 0;
        if (take(';')) separator = 1;
        else if (take(',')) {
            separator = 1;
            do { output(" "); } while (column % 14);
        }
        else break;
        spaces();
    }
    finished();
    if (!separator) newline();
}

static void dimension(void)
{
    char name[NAME_SIZE];
    uint8_t s, rank;
    uint16_t total, size;
    do {
        identifier(name); s = symbol(name);
        if (symbols[s].rank) fail("Array already dimensioned");
        need('('); rank = 0; total = 1;
        do {
            if (rank == 3) fail("Maximum array rank is 3");
            size = integer(floorf(numeric_expression() + .5f));
            if (size > CELLS - 1) fail("Invalid array bound");
            ++size;
            if (total > CELLS / size) fail("Array storage full");
            total *= size; symbols[s].dim[rank++] = size;
        } while (take(','));
        need(')');
        if (used_cells + total > CELLS) fail("Array storage full");
        /* The scalar cell allocated by symbol() remains unused. */
        symbols[s].base = used_cells; used_cells += total;
        symbols[s].rank = rank;
    } while (take(','));
    finished();
}

static void data_value(void)
{
    char *save = p, *save_end = end, *start, *s;
    uint16_t length;
    uint8_t negative;
    for (;;) {
        if (data_pc >= count) fail("Out of DATA");
        p = source + statements[data_pc].start;
        end = source + statements[data_pc].end;
        if (!keyword("DATA")) { ++data_pc; data_pos = 0; continue; }
        if (data_pos) p = source + data_pos;
        spaces();
        if (p == end) { ++data_pc; data_pos = 0; continue; }
        break;
    }
    if (*p == '"') {
        primary();
    } else {
        start = p;
        while (p < end && *p != ',') ++p;
        length = p - start;
        while (length && (start[length - 1] == ' ' || start[length - 1] == '\t'))
            --length;
        s = temporary(length); memcpy(s, start, length);
        /* Numeric DATA must consume its whole field, otherwise it is text. */
        p = start; negative = 0;
        if (*p == '+' || *p == '-') negative = *p++ == '-';
        if (digit(*p) || *p == '.') {
            push_number(literal());
            if (negative) values[vp - 1].n = -values[vp - 1].n;
            spaces();
            if (p != end && *p != ',') {
                --vp; push_string(s);
                p = start + length;
            }
        } else { push_string(s); p = start + length; }
    }
    spaces();
    if (p < end) { need(','); data_pos = p - source; }
    else { ++data_pc; data_pos = 0; }
    p = save; end = save_end;
}

static void read_statement(void)
{
    uint8_t sym;
    uint16_t cell;
    do {
        cell = variable(&sym);
        data_value(); store(cell, string_symbol(sym), --vp);
    } while (take(','));
    finished();
}

static void input_statement(void)
{
    uint16_t cell;
    uint8_t sym, i, c;
    char *save, *save_end, *start, *s, *inputp;
    spaces();
    if (*p == '#') fail("File I/O not supported");
    if (*p == '"') {
        primary(); output(text(--vp));
        if (!take(';')) need(',');
    }
    output("? ");
    i = 0;
    for (;;) {
        c = pending_key ? pending_key : util_call(1, 0);
        pending_key = 0;
        if (c == 3) fail("Break");
        if (c == '\r' || c == '\n') break;
        if (c == 8 || c == 127) { if (i) --i; }
        else {
            if (i == 126) fail("Input line too long");
            console_buffer[2 + i++] = c;
        }
    }
    newline();
    console_buffer[2 + i] = 0;
    inputp = (char *)console_buffer + 2;
    do {
        cell = variable(&sym);
        save = p; save_end = end;
        p = inputp; end = (char *)console_buffer + 2 + i;
        spaces();
        if (p == end) fail("Too few INPUT fields");
        if (string_symbol(sym)) {
            if (*p == '"') primary();
            else {
                start = p;
                while (p < end && *p != ',') ++p;
                s = temporary(p - start);
                memcpy(s, start, p - start); push_string(s);
            }
        } else { expression(1); (void)number(vp - 1); }
        store(cell, string_symbol(sym), --vp);
        spaces();
        c = p < end;
        if (c) need(',');
        inputp = p;
        p = save; end = save_end;
    } while (take(','));
    finished();
    if (c || *inputp) fail("Too many INPUT fields");
}

static void for_statement(void)
{
    uint8_t sym;
    uint16_t cell, bound;
    float initial;
    cell = variable(&sym);
    if (string_symbol(sym) || symbols[sym].rank) fail("FOR needs numeric scalar");
    need('='); initial = numeric_expression();
    if (!keyword("TO")) fail("Expected TO");
    bound = p - source;
    (void)numeric_expression();
    if (keyword("STEP")) (void)numeric_expression();
    finished();
    if (loop_count == DEPTH) fail("FOR nesting too deep");
    cells[cell].n = initial;
    loops[loop_count].cell = cell;
    loops[loop_count].body = pc + 1;
    loops[loop_count].bound = bound;
    loops[loop_count].finish = end - source;
    loops[loop_count++].calls = call_count;
    /* BASIC-E tests at NEXT: the body always executes at least once. */
}

static void next_statement(void)
{
    uint8_t sym;
    uint16_t cell;
    Loop *loop;
    char *save, *save_end;
    float limit, step;
    do {
        if (!loop_count) fail("NEXT without FOR");
        loop = &loops[loop_count - 1];
        if (loop->calls != call_count) fail("NEXT crosses GOSUB");
        spaces();
        if (p < end) {
            cell = variable(&sym);
            if (cell != loop->cell) fail("NEXT/FOR mismatch");
        }
        save = p; save_end = end;
        p = source + loop->bound; end = source + loop->finish;
        limit = numeric_expression();
        step = keyword("STEP") ? numeric_expression() : 1;
        finished(); p = save; end = save_end;
        cells[loop->cell].n = finite_number(cells[loop->cell].n + step);
        if (step == 0 || (step > 0 ? cells[loop->cell].n <= limit :
                                     cells[loop->cell].n >= limit)) {
            nextpc = loop->body;
            /* Validate any following NEXT names when they are reached. */
            return;
        }
        --loop_count;
    } while (take(','));
    finished();
}

static void gosub(uint16_t destination)
{
    if (call_count == DEPTH) fail("GOSUB nesting too deep");
    returns[call_count++] = pc + 1;
    nextpc = destination;
}

static void on_statement(void)
{
    int16_t selected = integer(numeric_expression()), index = 1;
    uint8_t subroutine;
    uint16_t destination = NONE;
    float n;
    if (keyword("GOSUB")) subroutine = 1;
    else if (keyword("GOTO")) subroutine = 0;
    else if (keyword("GO")) {
        if (!keyword("TO")) fail("Expected TO");
        subroutine = 0;
    } else { fail("Expected GOTO or GOSUB"); return; }
    do {
        n = numeric_expression();
        if (selected == index) destination = target(n);
        ++index;
    } while (take(','));
    finished();
    if (selected < 1 || selected >= index) fail("ON selector out of range");
    if (subroutine) gosub(destination); else nextpc = destination;
}

static void execute(void)
{
    uint8_t sym;
    uint16_t cell;
    float n;
    p = source + statements[pc].start;
    end = source + statements[pc].end;
    if (statements[pc].jump != NONE) {
        if (keyword("ELSE")) { nextpc = statements[pc].jump; return; }
        if (!keyword("IF")) fail("Invalid statement index");
        n = numeric_expression(); finished();
        if (!n) nextpc = statements[pc].jump;
        return;
    }
    if (keyword("PRINT") || take('?')) print_statement();
    else if (keyword("INPUT")) input_statement();
    else if (keyword("GOTO")) { n = numeric_expression(); finished(); nextpc = target(n); }
    else if (keyword("GO")) {
        if (!keyword("TO")) fail("Expected TO");
        n = numeric_expression(); finished(); nextpc = target(n);
    }
    else if (keyword("GOSUB")) { n = numeric_expression(); finished(); gosub(target(n)); }
    else if (keyword("RETURN")) {
        finished();
        if (!call_count) fail("RETURN without GOSUB");
        while (loop_count && loops[loop_count - 1].calls == call_count) --loop_count;
        nextpc = returns[--call_count];
    }
    else if (keyword("FOR")) for_statement();
    else if (keyword("NEXT")) next_statement();
    else if (keyword("ON")) on_statement();
    else if (keyword("DIM")) dimension();
    else if (keyword("READ")) read_statement();
    else if (keyword("RESTORE")) {
        spaces(); data_pc = p == end ? 0 : target(numeric_expression());
        data_pos = 0; finished();
    }
    else if (keyword("DATA")) return;
    else if (keyword("RANDOMIZE")) {
        spaces();
        rng = p == end ? rng + source_len + pc + util_call(11, 0) :
                        (uint16_t)integer(numeric_expression());
        finished();
    }
    else if (keyword("END") || keyword("STOP")) { finished(); nextpc = count; }
    else if (keyword("DEF") || keyword("FILE") || keyword("OPEN") ||
             keyword("CLOSE") || keyword("OUT") || keyword("POKE"))
        fail("Unsupported statement");
    else if (digit(*p)) { n = numeric_expression(); finished(); nextpc = target(n); }
    else {
        (void)keyword("LET");
        cell = variable(&sym);
        need('='); expression(1); finished();
        store(cell, string_symbol(sym), --vp);
    }
}

static uint16_t add_statement(uint16_t start, uint16_t stop, uint16_t label)
{
    uint16_t i;
    while (start < stop && (source[start] == ' ' || source[start] == '\t')) ++start;
    if (count == STATEMENTS) fail("Statement limit (256)");
    if (label)
        for (i = 0; i < count; ++i)
            if (statements[i].label == label) fail("Duplicate label");
    statements[count].start = start;
    statements[count].end = stop;
    statements[count].label = label;
    statements[count].jump = NONE;
    return count++;
}

static void index_source(void)
{
    uint16_t pos = 0, start, stop, line_end, first, label, id;
    uint16_t pending[DEPTH];
    uint8_t depth, quote;
    float label_value;
    while (pos < source_len) {
        first = count; depth = 0; label = 0;
        while (source[pos] == ' ' || source[pos] == '\t') ++pos;
        if (digit(source[pos])) {
            p = source + pos; end = source + source_len;
            label_value = literal();
            if (label_value < 1 || label_value > 65535 ||
                floorf(label_value) != label_value) fail("Invalid source label");
            label = (uint16_t)label_value; pos = p - source;
        }
        line_end = pos;
        while (line_end < source_len && source[line_end] != '\n') ++line_end;
        while (pos < line_end) {
            while (source[pos] == ' ' || source[pos] == '\t' || source[pos] == ':') ++pos;
            if (pos >= line_end) break;
            if (word_at(source + pos, "REM") || word_at(source + pos, "REMARK")) break;
            start = pos;
            if (word_at(source + pos, "ELSE")) {
                if (!depth) fail("ELSE without IF");
                id = add_statement(pos, pos + 4, label); label = 0;
                statements[pending[--depth]].jump = count;
                statements[id].jump = 0;
                pos += 4; continue;
            }
            if (word_at(source + pos, "IF")) {
                pos += 2; quote = 0;
                while (pos < line_end) {
                    if (source[pos] == '"') quote = !quote;
                    if (!quote && word_at(source + pos, "THEN") &&
                        !namechar(source[pos - 1])) break;
                    ++pos;
                }
                if (pos == line_end) fail("IF without THEN");
                if (depth == DEPTH) fail("IF nesting too deep");
                id = add_statement(start, pos, label); label = 0;
                statements[id].jump = 0; pending[depth++] = id;
                pos += 4; continue;
            }
            quote = 0;
            while (pos < line_end) {
                if (source[pos] == '"') quote = !quote;
                if (!quote) {
                    if (source[pos] == ':') break;
                    if (pos == start || !namechar(source[pos - 1])) {
                        if (word_at(source + pos, "ELSE") ||
                            word_at(source + pos, "REM") ||
                            word_at(source + pos, "REMARK")) break;
                    }
                }
                ++pos;
            }
            if (quote) fail("Unterminated string");
            stop = pos;
            if (stop > start) {
                (void)add_statement(start, stop, label); label = 0;
            }
        }
        /* A labelled REM/empty line is a valid branch destination. */
        if (label) (void)add_statement(line_end, line_end, label);
        for (id = first; id < count; ++id)
            if (statements[id].jump == 0) statements[id].jump = count;
        pos = line_end + 1;
    }
}

static void load_source(void)
{
    uint8_t length, pos = 0, i, c, status;
    uint16_t fcb = util_fcb_address(0), dma = util_dma_buffer(), trim;
    uint8_t done = 0, previous_cr = 0;
    length = util_args(arguments);
    while (pos < length && arguments[pos] == ' ') ++pos;
    if (pos == length) fail("Usage: BASIC filename[.BAS]");
    if (!util_parse_fcb(arguments, length, &pos, fcb, 0, 0))
        fail("Invalid filename");
    while (pos < length && arguments[pos] == ' ') ++pos;
    if (pos != length) fail("Unexpected command arguments");
    if (rb(fcb + 9) == ' ') {
        wb(fcb + 9, 'B'); wb(fcb + 10, 'A'); wb(fcb + 11, 'S');
    }
    if (util_call(15, fcb) == 255) fail("Cannot open source file");
    util_set_dma(dma);
    while (!done) {
        status = util_call(20, fcb);
        if (status == 1) break;
        if (status) { util_call(16, fcb); fail("Source read error"); }
        for (i = 0; i < 128; ++i) {
            c = rb(dma + i);
            if (c == 26) { done = 1; break; }
            if (!c) { util_call(16, fcb); fail("NUL in source"); }
            if (c == '\n' && previous_cr) { previous_cr = 0; continue; }
            previous_cr = c == '\r';
            if (c == '\r' || c == '\n') {
                trim = source_len;
                while (trim && (source[trim - 1] == ' ' || source[trim - 1] == '\t'))
                    --trim;
                if (trim && source[trim - 1] == '\\') {
                    source_len = trim; source[trim - 1] = ' '; continue;
                }
                c = '\n';
            }
            if (source_len >= SOURCE_SIZE - 2) {
                util_call(16, fcb); fail("Source exceeds 6142 bytes");
            }
            source[source_len++] = c;
        }
    }
    if (util_call(16, fcb) == 255) fail("Source close error");
    source[source_len++] = '\n'; source[source_len] = 0;
}

void main(void)
{
    /* Startup zeroes _DATA; no initialized writable globals are required. */
    if (setjmp(abort_run)) return;
    rng = 1;
    load_source();
    index_source();
    for (pc = 0; pc < count; pc = nextpc) {
        nextpc = pc + 1;
        scratch_used = 0; vp = 0; recursion = 0;
        p = source + statements[pc].start;
        end = source + statements[pc].end;
        spaces();
        if (p == end) continue;
        if (!pending_key && util_call(11, 0)) {
            pending_key = util_call(1, 0);
            if (pending_key == 3) fail("Break");
        }
        execute();
    }
}
