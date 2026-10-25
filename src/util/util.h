#ifndef CPM_UTIL_H
#define CPM_UTIL_H

#include <stdint.h>

#define UTIL_TAIL       0x0080u
#define UTIL_EOF        0x1Au
#define UTIL_CR         13u
#define UTIL_LF         10u

uint8_t util_args(uint8_t *args);
uint8_t util_parse_fcb(const uint8_t *text, uint8_t length, uint8_t *pos,
                       uint16_t fcb, uint8_t default_drive,
                       uint8_t wildcards);
void util_clear_fcb(uint16_t fcb);
uint8_t util_call(uint8_t function, uint16_t address);
uint16_t util_call_pointer(uint8_t function, uint16_t address);
uint16_t bdos_pointer_call(uint8_t function, uint16_t address);
void util_select(uint8_t drive);
void util_set_dma(uint16_t address);
void util_print(const char *text);
void util_print_char(uint8_t value);
void util_print_newline(void);
void util_print_hex(uint8_t value);
void util_print_decimal(uint16_t value);
void util_print_filename(uint16_t fcb);
uint16_t util_pointer(const void *pointer);
uint16_t util_dma_buffer(void);
uint16_t util_fcb_address(uint8_t index);
uint8_t util_current_drive(void);

#endif
