/**
 * @file uart_reg.h
 * @brief UART peripheral and pin pair tables for the STM32G4
 *
 * Register definitions come from the HAL / CMSIS device header, so this file
 * holds only the peripheral pointer table, the valid TX/RX pin pairs with
 * their alternate function, and the driver's timeouts. Included only by uart.c.
 *
 * Reference: STM32G431xx datasheet DS12589, Table 13 (alternate functions)
 */

#ifndef UART_REG_H
#define UART_REG_H

#include <stdint.h>
#include "stm32g4xx_hal.h"
#include "gpio_platform.h"


// uart_num indexes this table
static USART_TypeDef *const uart_instances[] = {
    USART1,     // 0
    USART2,     // 1
    USART3,     // 2
    UART4,      // 3
    LPUART1,    // 4
};

#define NUM_UART_PERIPHERALS (sizeof(uart_instances) / sizeof(uart_instances[0]))

typedef struct {
    uint8_t tx;
    uint8_t rx;
    uint8_t af;
    uint8_t peripheral;
} uart_pin_pair;

// Valid UART pin pairs. First match wins, so PA2/PA3 resolves to USART2
// (AF7) rather than LPUART1 (AF12); both reach the Nucleo ST-LINK VCP.
// SWD pins (PA13/PA14) and their shared pairs are left out on purpose.
static const uart_pin_pair uart_valid_pairs[] = {
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_A,  9), .rx = BM_GPIO_PIN(BM_GPIO_PORT_A, 10), .af = GPIO_AF7_USART1,  .peripheral = 0 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_B,  6), .rx = BM_GPIO_PIN(BM_GPIO_PORT_B,  7), .af = GPIO_AF7_USART1,  .peripheral = 0 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_C,  4), .rx = BM_GPIO_PIN(BM_GPIO_PORT_C,  5), .af = GPIO_AF7_USART1,  .peripheral = 0 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_A,  2), .rx = BM_GPIO_PIN(BM_GPIO_PORT_A,  3), .af = GPIO_AF7_USART2,  .peripheral = 1 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_B,  3), .rx = BM_GPIO_PIN(BM_GPIO_PORT_B,  4), .af = GPIO_AF7_USART2,  .peripheral = 1 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_B, 10), .rx = BM_GPIO_PIN(BM_GPIO_PORT_B, 11), .af = GPIO_AF7_USART3,  .peripheral = 2 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_C, 10), .rx = BM_GPIO_PIN(BM_GPIO_PORT_C, 11), .af = GPIO_AF7_USART3,  .peripheral = 2 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_B, 11), .rx = BM_GPIO_PIN(BM_GPIO_PORT_B, 10), .af = GPIO_AF8_LPUART1, .peripheral = 4 },
    { .tx = BM_GPIO_PIN(BM_GPIO_PORT_C,  1), .rx = BM_GPIO_PIN(BM_GPIO_PORT_C,  0), .af = GPIO_AF8_LPUART1, .peripheral = 4 },
};
// UART4 shares PC10/PC11 with USART3 (AF5 vs AF7); USART3 wins the lookup,
// so UART4 is reachable only once a distinct pin pair is added here.

#define NUM_UART_PAIRS (sizeof(uart_valid_pairs) / sizeof(uart_pin_pair))

// per-byte timeouts in milliseconds, measured by the SysTick-backed HAL_GetTick
// TX: one frame at the slowest baud HAL accepts is a few ms
#define UART_TX_TIMEOUT_MS  100U
// RX: how long bm_uart_read_byte waits for a byte to arrive
#define UART_RX_TIMEOUT_MS  2000U

#endif /* UART_REG_H */
