/**
 * @file    rcc_clock_init.h
 * @brief   STM32G431RB system clock configuration.
 *
 * Configures SYSCLK to 144 MHz by driving HSI16 through the main PLL
 * (M = 4, N = 72, R = 2), sourced entirely from RM0440's increasing-
 * frequency sequence (flash latency, PLL setup, PLL enable, SYSCLK
 * switch). Target chosen for margin under the 150 MHz Range 1 normal
 * mode ceiling against HSI16 temperature drift.
 *
 */

#ifndef RCC_CLOCK_INIT_H
#define RCC_CLOCK_INIT_H

#include <stdbool.h>
#include <stdint.h>

//confirmed in implementation, defined as constant here for use in compile time constants
#define BM_RCC_SYSCLK_HZ 144000000UL 

/**
 * @brief   Bring up SYSCLK at 144 MHz from HSI16 via the main PLL.
 *
 * Must be called once, at the start of main(), before any peripheral
 * relies on core or bus clock timing (SysTick, UART, SPI, etc.).
 * Returns true if the clock was successfully configured, false if
 * the configuration failed (e.g., PLL lock timeout).
 */
bool bm_rcc_clock_init(void);

#endif /* RCC_CLOCK_INIT_H */