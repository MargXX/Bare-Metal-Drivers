/**
 * @file i2c_reg.h
 * @brief I2C peripheral, pin pair, and timing tables for the STM32G4
 *
 * Register definitions come from the HAL / CMSIS device header, so this file
 * holds only the peripheral pointer table, the valid SDA/SCL pin pairs with
 * their alternate function, the precomputed TIMINGR values, and the driver's
 * timeout. Included only by i2c.c.
 *
 * Reference: STM32G431xx datasheet DS12589, Table 13 (alternate functions)
 *            RM0440 §37.4.9 (I2C timings), §37.7.5 (I2C_TIMINGR)
 *            NXP UM10204 Table 10 (I2C bus timing limits)
 */

#ifndef I2C_REG_H
#define I2C_REG_H

#include <stdint.h>
#include "stm32g4xx_hal.h"
#include "gpio_platform.h"
#include "rcc.h"


// i2c_num indexes this table
static I2C_TypeDef *const i2c_instances[] = {
    I2C1,   // 0
    I2C2,   // 1
    I2C3,   // 2
};

#define NUM_I2C_PERIPHERALS (sizeof(i2c_instances) / sizeof(i2c_instances[0]))

typedef struct {
    uint8_t sda;
    uint8_t scl;
    uint8_t af;
    uint8_t peripheral;
} i2c_pin_pair;

// Valid I2C pin pairs. PB8/PB9 is the Nucleo Arduino D15/D14 header.
// SWD pins (PA13/PA14) and their shared pairs are left out on purpose.
static const i2c_pin_pair i2c_valid_pairs[] = {
    { .sda = BM_GPIO_PIN(BM_GPIO_PORT_B, 9), .scl = BM_GPIO_PIN(BM_GPIO_PORT_B,  8), .af = GPIO_AF4_I2C1, .peripheral = 0 },
    { .sda = BM_GPIO_PIN(BM_GPIO_PORT_B, 7), .scl = BM_GPIO_PIN(BM_GPIO_PORT_A, 15), .af = GPIO_AF4_I2C1, .peripheral = 0 },
    { .sda = BM_GPIO_PIN(BM_GPIO_PORT_A, 8), .scl = BM_GPIO_PIN(BM_GPIO_PORT_A,  9), .af = GPIO_AF4_I2C2, .peripheral = 1 },
    { .sda = BM_GPIO_PIN(BM_GPIO_PORT_C, 9), .scl = BM_GPIO_PIN(BM_GPIO_PORT_C,  8), .af = GPIO_AF8_I2C3, .peripheral = 2 },
};

#define NUM_I2C_PAIRS (sizeof(i2c_valid_pairs) / sizeof(i2c_pin_pair))


// ------------------------------------------------------------------
// TIMINGR values, I2CCLK = PCLK1 = 144 MHz (RCC_CCIPR.I2CxSEL reset value
// selects PCLK1), analog filter on, digital filter off.
//
// t_PRESC = (PRESC + 1) / I2CCLK
// t_LOW   = (SCLL + 1) * t_PRESC      t_HIGH  = (SCLH + 1) * t_PRESC
// t_SCLDEL = (SCLDEL + 1) * t_PRESC  >= t_r + t_SU;DAT(min)
// t_SDADEL = SDADEL * t_PRESC        >= t_f + t_HD;DAT(min) - t_AF(min) - 3 * t_I2CCLK
//
// t_SDADEL <= t_HD;DAT(max) - t_r - t_AF(max) - 4 * t_I2CCLK
// t_SCL    =  t_LOW + t_HIGH + t_SYNC1 + t_SYNC2, where each t_SYNC is at
//             least t_AF(min) 50 ns + 2 * t_I2CCLK, so ~130 ns total
//
// Standard mode, 100 kHz: PRESC 15 -> t_PRESC 111.1 ns
//   SCLL 46  -> t_LOW 5.22 us  (min 4.7 us)
//   SCLH 41  -> t_HIGH 4.67 us (min 4.0 us)
//   period >= 9.89 + 0.13 us = 10.02 us -> at most 99.8 kHz
//   SCLDEL 11 -> 1.33 us >= 1000 ns t_r + 250 ns t_SU;DAT
//   SDADEL 3  -> 333 ns, within [229 ns, 2162 ns]
//
// Fast mode, 400 kHz: PRESC 5 -> t_PRESC 41.67 ns
//   SCLL 33  -> t_LOW 1.42 us  (min 1.3 us)
//   SCLH 23  -> t_HIGH 1.00 us (min 0.6 us)
//   period >= 2.42 + 0.13 us = 2.55 us -> at most 392 kHz
//   SCLDEL 9  -> 417 ns  >= 300 ns t_r + 100 ns t_SU;DAT
//   SDADEL 6  -> 250 ns, within [229 ns, 312 ns]
//
// Rise time on the real bus lengthens the period further, so the bus runs
// under the nominal rate, never over it.
// ------------------------------------------------------------------
#define I2C_TIMINGR(presc, scldel, sdadel, sclh, scll) \
    (((uint32_t)(presc) << I2C_TIMINGR_PRESC_Pos)   | \
     ((uint32_t)(scldel) << I2C_TIMINGR_SCLDEL_Pos) | \
     ((uint32_t)(sdadel) << I2C_TIMINGR_SDADEL_Pos) | \
     ((uint32_t)(sclh) << I2C_TIMINGR_SCLH_Pos)     | \
     ((uint32_t)(scll) << I2C_TIMINGR_SCLL_Pos))

#define I2C_TIMINGR_STANDARD  I2C_TIMINGR(15, 11, 3, 41, 46)
#define I2C_TIMINGR_FAST      I2C_TIMINGR(5, 9, 6, 23, 33)

_Static_assert(BM_RCC_SYSCLK_HZ == 144000000UL,
               "I2C TIMINGR values are computed for a 144 MHz I2CCLK; recompute them");

// per-transaction timeout in milliseconds, measured by the SysTick-backed HAL_GetTick
// HAL applies it to each flag wait, so a long transfer is not cut off partway
#define I2C_TIMEOUT_MS  100U

#endif /* I2C_REG_H */
