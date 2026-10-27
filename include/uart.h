/**
 * @file uart.h
 * @brief Portable UART transfer API.
 *
 * Declares the UART operations whose signature and semantics are the same on
 * every target: byte and buffer transfer, string transmit, and non-blocking
 * FIFO status checks.
 *
 * Initialization and register-level status are not portable and are declared
 * in the target's uart_platform.h, which this header pulls in so callers need
 * only one include. CMake selects which target's platform header resolves.
 *
 * uart_num identifies an initialized peripheral. It is produced by the
 * platform's bm_uart_init and has no meaning before that call.
 *
 * All functions return true on success, false on error.
 */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart_platform.h"

// transmit a single byte — blocks until TX FIFO has space
bool bm_uart_write_byte(uint8_t uart_num, uint8_t byte);

// transmit len bytes from buf — blocks until all bytes are written
bool bm_uart_write(uint8_t uart_num, const uint8_t *buf, size_t len);

// transmit a null-terminated string — blocks until all bytes are written
bool bm_uart_write_str(uint8_t uart_num, const char *str);

// receive a single byte into byte_out — blocks until RX FIFO has data
bool bm_uart_read_byte(uint8_t uart_num, uint8_t *byte_out);

// receive up to len bytes into buf — blocks until len bytes are received
bool bm_uart_read(uint8_t uart_num, uint8_t *buf, size_t len);

// returns true if the TX FIFO is full — use to check before writing if non-blocking behavior is needed
bool bm_uart_tx_full(uint8_t uart_num);

// returns true if the RX FIFO has data available — use to poll before reading
bool bm_uart_rx_ready(uint8_t uart_num);

//add functions later to adjust settings like baud rate, parity, stop bits, or to disable/enable interrupts as needed

#endif /* UART_H */