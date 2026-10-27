/**
 * @file uart_platform.h
 * @brief Public platform constants for RP2040 UART
 */


#ifndef UART_PLATFORM_H

#include <stdint.h>


// initialize UART peripheral — must be called before any other UART functions
// tx_pin and rx_pin must be a valid UART-capable GPIO pair for the target peripheral
// returns false if pin combination is invalid or peripheral is already enabled
bool bm_uart_init(uint8_t *uart_num, uint32_t baud_rate, uint8_t tx_pin, uint8_t rx_pin);


// reads UARTRIS (raw interrupt status) and UARTFR (flag register) into status_out and flags_out
// useful for diagnosing framing errors, overrun, break conditions, or FIFO state
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *flags_out);

#endif /* UART_PLATFORM_H */