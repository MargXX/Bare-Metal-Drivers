/**
 * @file uart.h
 * @brief UART driver API for the RP2040
 *
 * Provides bare-metal UART communication using direct register access.
 * No HAL abstraction — all configuration is written against the RP2040 reference manual.
 *
 * Clock assumption: 125MHz processor clock (set by pico_runtime on startup).
 *   Baud rate is derived from UARTIBRD and UARTFBRD — update both in uart.c
 *   if changing baud rate or clock source.
 *
 * Pin assumption: GP0 (TX) and GP1 (RX) using UART1 peripheral.
 *   Update UART_BASE and GPIO pin assignments in uart_reg.h if using UART0
 *   or alternate pins.
 *
 * Reference: RP2040 Datasheet, Section 4.2
 */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart_reg.h"

// returns false on error, true on success

// initialize UART peripheral — must be called before any other UART functions
// configures GPIO pins, baud rate divisors, line format (8N1), and enables TX/RX
bool bm_uart_init(uint32_t baud_rate);

// transmit a single byte — blocks until TX FIFO has space
bool bm_uart_write_byte(uint8_t byte);

// transmit len bytes from buf — blocks until all bytes are written
bool bm_uart_write(const uint8_t *buf, size_t len);

// transmit a null-terminated string — blocks until all bytes are written
bool bm_uart_write_str(const char *str);

// receive a single byte into byte_out — blocks until RX FIFO has data
bool bm_uart_read_byte(uint8_t *byte_out);

// receive up to len bytes into buf — blocks until len bytes are received
bool bm_uart_read(uint8_t *buf, size_t len);

// returns true if the TX FIFO is full — use to check before writing if non-blocking behavior is needed
bool bm_uart_tx_full(void);

// returns true if the RX FIFO has data available — use to poll before reading
bool bm_uart_rx_ready(void);

// reads UARTRIS (raw interrupt status) and UARTFR (flag register) into status_out and flags_out
// useful for diagnosing framing errors, overrun, break conditions, or FIFO state
bool bm_uart_get_status(uint32_t *status_out, uint32_t *flags_out);

#endif /* UART_H */