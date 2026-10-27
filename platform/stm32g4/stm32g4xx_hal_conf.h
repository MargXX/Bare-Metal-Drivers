/**
 * @file stm32g4xx_hal_conf.h
 * @brief HAL module selection for the STM32G4 peripheral drivers
 *
 * Derived from ST's stm32g4xx_hal_conf_template.h (HAL v1.2.7). Only the
 * modules the GPIO, UART, and I2C drivers depend on are enabled. Enable more
 * here as drivers are added (e.g. HAL_SPI_MODULE_ENABLED).
 *
 * The HAL is a backend only: the clock tree is owned by bm_rcc_clock_init and
 * the SysTick timebase by platform/cortex-m/systick. See hal_glue.c for how
 * the HAL is pointed at both.
 */

#ifndef STM32G4xx_HAL_CONF_H
#define STM32G4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------
// Module selection
// ------------------------------------------------------------------
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED      // UART and I2C handles carry DMA handle pointers
#define HAL_FLASH_MODULE_ENABLED    // RCC HAL reads flash latency macros
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED      // RCC extended HAL references PWR
#define HAL_RCC_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

// ------------------------------------------------------------------
// Register callbacks — all off, drivers are polling only
// ------------------------------------------------------------------
#define USE_HAL_I2C_REGISTER_CALLBACKS        0U
#define USE_HAL_UART_REGISTER_CALLBACKS       0U

// ------------------------------------------------------------------
// Oscillator values
// HSI_VALUE is what HAL_RCC_GetSysClockFreq uses to compute PCLK from the
// PLL settings bm_rcc_clock_init writes. HSE (24 MHz crystal on the
// Nucleo-G431RB) and LSE are not used, but the HAL headers require the macros.
// ------------------------------------------------------------------
#if !defined(HSE_VALUE)
#define HSE_VALUE               (24000000UL)
#endif
#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT     (100UL)
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE               (16000000UL)
#endif
#if !defined(HSI48_VALUE)
#define HSI48_VALUE             (48000000UL)
#endif
#if !defined(LSI_VALUE)
#define LSI_VALUE               (32000UL)
#endif
#if !defined(LSE_VALUE)
#define LSE_VALUE               (32768UL)
#endif
#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT     (5000UL)
#endif
#if !defined(EXTERNAL_CLOCK_VALUE)
#define EXTERNAL_CLOCK_VALUE    (48000UL)
#endif

// ------------------------------------------------------------------
// System configuration
// TICK_INT_PRIORITY is never applied: HAL_InitTick is overridden in
// hal_glue.c so the HAL does not reprogram SysTick.
// ------------------------------------------------------------------
#define VDD_VALUE                    (3300UL)
#define TICK_INT_PRIORITY            (0x0FUL)
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              0U
#define INSTRUCTION_CACHE_ENABLE     1U
#define DATA_CACHE_ENABLE            1U

#define USE_SPI_CRC                  0U

// ------------------------------------------------------------------
// Module headers
// ------------------------------------------------------------------
#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32g4xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32g4xx_hal_gpio.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32g4xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32g4xx_hal_cortex.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32g4xx_hal_flash.h"
#endif
#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32g4xx_hal_i2c.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32g4xx_hal_pwr.h"
#endif
#ifdef HAL_UART_MODULE_ENABLED
#include "stm32g4xx_hal_uart.h"
#endif

// assert_param compiled out; drivers validate their own inputs and return false
#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif

#endif /* STM32G4xx_HAL_CONF_H */
