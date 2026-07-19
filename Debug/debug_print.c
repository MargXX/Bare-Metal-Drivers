/**
 * @file debug_print.c
 * @brief Formatted debug output over UART (hex and decimal), for test files.
 */

#include <stddef.h>
#include "debug_print.h"
#include "../UART/uart.h"

static const char HEX_DIGITS[] = "0123456789ABCDEF";

// write `digits` hex characters of value into buf, most significant first.
// does not null-terminate, caller controls buf sizing.
static void hex_digits_to_buf(uint32_t value, uint8_t digits, char *buf) {
    for (uint8_t i = 0; i < digits; i++) {
        uint8_t shift = (uint8_t)((digits - 1 - i) * 4);
        buf[i] = HEX_DIGITS[(value >> shift) & 0xF];
    }
}

// shared hex printer for any width. digits is the number of hex characters.
static void print_hex(uint8_t uart_num, uint32_t value, uint8_t digits) {
    char buf[2 + 8 + 2]; // "0x" + up to 8 hex digits + \r\n, sized for the widest case (hex32)
    buf[0] = '0';
    buf[1] = 'x';
    hex_digits_to_buf(value, digits, &buf[2]);
    buf[2 + digits]     = '\r';
    buf[2 + digits + 1] = '\n';
    bm_uart_write(uart_num, (const uint8_t *)buf, (size_t)(2 + digits + 2));
}

void bm_debug_print_hex8(uint8_t uart_num, uint8_t value) {
    print_hex(uart_num, value, 2);
}

void bm_debug_print_hex16(uint8_t uart_num, uint16_t value) {
    print_hex(uart_num, value, 4);
}

void bm_debug_print_hex32(uint8_t uart_num, uint32_t value) {
    print_hex(uart_num, value, 8);
}

void bm_debug_print_dec32(uint8_t uart_num, uint32_t value) {
    char digits_buf[10]; // max digits in a uint32_t (4294967295)
    uint8_t count = 0;

    if (value == 0) {
        digits_buf[count++] = '0';
    } else {
        while (value > 0) {
            digits_buf[count++] = (char)('0' + (value % 10));
            value /= 10;
        }
    }

    // digits_buf is least-significant-first, write it out reversed, then \r\n
    char out_buf[10 + 2];
    for (uint8_t i = 0; i < count; i++) {
        out_buf[i] = digits_buf[count - 1 - i];
    }
    out_buf[count]     = '\r';
    out_buf[count + 1] = '\n';
    bm_uart_write(uart_num, (const uint8_t *)out_buf, (size_t)(count + 2));
}

// print a signed 32-bit integer as decimal digits + \r\n (no leading zeros)
void bm_debug_print_dec32_signed(uint8_t uart_num, int32_t value) {
    // print a signed 32-bit integer as decimal digits + \r\n (no leading zeros)
    char digits_buf[10];   // max digits of uint32_t magnitude
    char out_buf[11 + 2];  // optional '-' + 10 digits + CRLF

    uint8_t count = 0;
    bool is_negative = true;
    uint32_t magnitude;

    if (value < 0) {
        is_negative = true;
        magnitude = (uint32_t)(-(value + 1)) + 1; // handles INT32_MIN safely
    } else {
        magnitude = (uint32_t)value;
    }

    if (magnitude == 0) {
        digits_buf[count++] = '0';
    } else {
        while (magnitude > 0) {
            digits_buf[count++] = (char)('0' + (magnitude % 10));
            magnitude /= 10;
        }
    }

    uint8_t out_count = 0;

    if (is_negative) {
        out_buf[out_count++] = '-';
    }

    for (uint8_t i = 0; i < count; i++) {
        out_buf[out_count++] = digits_buf[count - 1 - i];
    }

    out_buf[out_count++] = '\r';
    out_buf[out_count++] = '\n';

    bm_uart_write(uart_num,
                  (const uint8_t *)out_buf,
                  (size_t)out_count);
}

void bm_debug_print_bool(uint8_t uart_num, bool value) {
    bm_uart_write_str(uart_num, value ? "true\r\n" : "false\r\n");
}

void bm_debug_print_labeled_hex8(uint8_t uart_num, const char *label, uint8_t value) {
    bm_uart_write_str(uart_num, label);
    bm_uart_write_str(uart_num, ": ");
    bm_debug_print_hex8(uart_num, value);
}

void bm_debug_print_result(uint8_t uart_num, bool passed, const char *label) {
    bm_uart_write_str(uart_num, passed ? "[PASS] " : "[FAIL] ");
    bm_uart_write_str(uart_num, label);
    bm_uart_write_str(uart_num, "\r\n");
}