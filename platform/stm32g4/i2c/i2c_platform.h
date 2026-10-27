/**
 * @file i2c_platform.h
 * @brief Public platform declarations for STM32G4 I2C
 *
 * Pins are packed port/pin values built with BM_GPIO_PIN (gpio_platform.h).
 */


#ifndef I2C_PLATFORM_H
#define I2C_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include "gpio_platform.h"


// I2C clock speed — TIMINGR values for each mode are precomputed in i2c_reg.h
// Standard mode (100kHz): I2C_MODE_STANDARD
// Fast mode (400kHz):     I2C_MODE_FAST
// Fast mode plus is not supported on this target yet (needs SYSCFG FMP drive
// and its own TIMINGR); bm_i2c_init rejects clock_hz above I2C_MODE_FAST.
#define I2C_MODE_STANDARD   100000U
#define I2C_MODE_FAST       400000U


// initialize I2C peripheral in master mode — must be called before any other I2C functions
// configures GPIO pins (open drain, internal pull-up), timing, and enables peripheral
// bm_rcc_clock_init and bm_systick_init must have run first
bool bm_i2c_init(uint8_t *i2c_num, uint32_t clock_hz, uint8_t sda_pin, uint8_t scl_pin);


// reads I2C_ISR and the HAL error code of the most recent transaction into
// status_out and error_out
// useful for diagnosing NAK (HAL_I2C_ERROR_AF), arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint8_t i2c_num, uint32_t *status_out, uint32_t *error_out);

#endif /* I2C_PLATFORM_H */
