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
 *   Pin configuration: TX and RX pins are passed to bm_uart_init at runtime.
 *   UART peripheral is inferred from the pin pair — see uart_reg.h for valid combinations.
 * Reference: RP2040 Datasheet, Section 4.2
 */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// returns false on error, true on success

// initialize UART peripheral — must be called before any other UART functions
// tx_pin and rx_pin must be a valid UART-capable GPIO pair for the target peripheral
// returns false if pin combination is invalid or peripheral is already enabled
bool bm_uart_init(uint8_t *uart_num, uint32_t baud_rate, uint8_t tx_pin, uint8_t rx_pin);

// transmit a single byte — blocks until TX FIFO has space
bool bm_uart_write_byte(uint8_t uart_num,uint8_t byte);

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

// reads UARTRIS (raw interrupt status) and UARTFR (flag register) into status_out and flags_out
// useful for diagnosing framing errors, overrun, break conditions, or FIFO state
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *flags_out);

//add functions later to adjust settings like baud rate, parity, stop bits, or to disable/enable interrupts as needed


#endif /* UART_H */