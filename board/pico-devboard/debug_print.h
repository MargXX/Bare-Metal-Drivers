/**
 * @file debug_print.h
 * @brief Formatted debug output over UART (hex and decimal), for test files.
 *
 * Presentation-layer helpers only. Depends on the UART driver's public API
 * (bm_uart_write_str / bm_uart_write_byte) but is not part of the UART
 * driver itself, so it can be dropped or swapped without touching uart.c.
 */

#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdint.h>
#include <stdbool.h>

// print an 8-bit value as "0x" + 2 hex digits + \r\n
void bm_debug_print_hex8(uint8_t uart_num, uint8_t value);

// print a 16-bit value as "0x" + 4 hex digits + \r\n
void bm_debug_print_hex16(uint8_t uart_num, uint16_t value);

// print a 32-bit value as "0x" + 8 hex digits + \r\n
void bm_debug_print_hex32(uint8_t uart_num, uint32_t value);

// print a 32-bit value as decimal digits + \r\n (no leading zeros)
void bm_debug_print_dec32(uint8_t uart_num, uint32_t value);

// print a signed 32-bit integer as decimal digits + \r\n (no leading zeros)
void bm_debug_print_dec32_signed(uint8_t uart_num, int32_t value);

// print a bool as "true" or "false" + \r\n
void bm_debug_print_bool(uint8_t uart_num, bool value);

// print a label string followed by ": " then a hex8 value, all on one line
void bm_debug_print_labeled_hex8(uint8_t uart_num, const char *label, uint8_t value);

// print a [PASS]/[FAIL] tag followed by a label string + \r\n
void bm_debug_print_result(uint8_t uart_num, bool passed, const char *label);

#endif // DEBUG_PRINT_H