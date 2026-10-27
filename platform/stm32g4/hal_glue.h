/**
 * @file hal_glue.h
 * @brief Internal checks shared by the STM32G4 HAL-backed drivers
 *
 * Included only by platform/stm32g4 driver sources. Not a caller-facing header.
 */

#ifndef HAL_GLUE_H
#define HAL_GLUE_H

#include <stdbool.h>

// returns true if SysTick is enabled with its interrupt on, i.e. bm_systick_init
// has run. Every HAL timeout reads HAL_GetTick, which is backed by the SysTick
// millisecond count, so a driver must not call into the HAL before this holds.
bool bm_hal_timebase_ready(void);

// returns true if PCLK1 and PCLK2 are running at BM_RCC_SYSCLK_HZ, i.e.
// bm_rcc_clock_init has run. Peripheral timing (I2C TIMINGR) is precomputed
// for that frequency.
bool bm_hal_clock_ready(void);

#endif /* HAL_GLUE_H */
