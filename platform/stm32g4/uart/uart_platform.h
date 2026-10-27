/**
 * @file uart_platform.h
 * @brief Public platform declarations for STM32G4 UART
 *
 * Pins are packed port/pin values built with BM_GPIO_PIN (gpio_platform.h).
 */


#ifndef UART_PLATFORM_H
#define UART_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include "gpio_platform.h"


// initialize UART peripheral as 8N1 — must be called before any other UART functions
// tx_pin and rx_pin must be a valid UART-capable pin pair (see uart_reg.h)
// bm_rcc_clock_init and bm_systick_init must have run first
// returns false if pin combination is invalid, the timebase is not running,
// or the HAL rejects the baud rate for this peripheral's clock
bool bm_uart_init(uint8_t *uart_num, uint32_t baud_rate, uint8_t tx_pin, uint8_t rx_pin);


// reads USART_ISR (interrupt and status register) and the HAL error code into
// status_out and error_out
// useful for diagnosing framing errors, overrun, noise, or parity errors
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *error_out);

#endif /* UART_PLATFORM_H */
