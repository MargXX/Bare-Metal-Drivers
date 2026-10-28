/**
 * @file i2c.h
 * @brief Portable I2C master transfer API.
 *
 * Declares the I2C master operations whose signature and semantics are the
 * same on every target: write, read, combined write-read with a repeated
 * START, bus-busy polling, peripheral recovery, and handle validation.
 *
 * Initialization and register-level status are not portable and are declared
 * in the target's i2c_platform.h, along with the bus speed constants, whose
 * register encodings are vendor-specific. This header pulls that one in so
 * callers need only one include; CMake selects which target resolves.
 *
 * i2c_num identifies an initialized peripheral. It is produced by the
 * platform's bm_i2c_init and has no meaning before that call.
 *
 * Device addresses are 7-bit and right-aligned. The direction bit is supplied
 * by the driver and must not be included by the caller.
 *
 * All functions return true on success, false on error.
 */

#ifndef I2C_H
#define I2C_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_platform.h"

// transmit len bytes from buf to device at 7-bit addr
// generates START, sends address + write bit, sends data, generates STOP
bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len);

// receive len bytes from device at 7-bit addr into buf
// generates START, sends address + read bit, clocks in data, generates STOP
bool bm_i2c_read(uint8_t i2c_num, uint8_t addr, uint8_t *buf, size_t len);

// write reg_addr, then read len bytes into buf without releasing the bus
// issues a repeated START between write and read phases
// this is the correct pattern for sensor register reads (BMP390, LSM9DS1)
bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len);

// returns true if the I2C bus or peripheral is currently busy
// check before initiating a transfer to avoid collisions
bool bm_i2c_is_busy(uint8_t i2c_num);

// disable and re-enable the I2C peripheral to recover from a hung bus
// use if SDA or SCL lines are stuck and normal transactions are not completing
bool bm_i2c_reset(uint8_t i2c_num);

// for outside functions checking validity of i2c_num within other drivers like the BMP390 driver
bool bm_i2c_is_valid(uint8_t i2c_num);

//add functions later to adjust settings like clock speed, addressing mode, or to disable/enable interrupts as needed

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */