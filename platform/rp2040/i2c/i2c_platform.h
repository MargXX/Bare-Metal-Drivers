/**
 * @file i2c_platform.h
 * @brief Public platform constants for RP2040 I2C
 */


// I2C clock speed — update SCL_HCNT/LCNT in i2c.c if changing
// Standard mode (100kHz): I2C_MODE_STANDARD
// Fast mode (400kHz):     I2C_MODE_FAST
#define I2C_MODE_STANDARD   100000U
#define I2C_MODE_FAST       400000U
#define I2C_MODE_FAST_PLUS  1000000U


// initialize I2C peripheral in master mode — must be called before any other I2C functions
// configures GPIO pins, clock divider, and enables peripheral
bool bm_i2c_init(uint8_t *i2c_num, uint32_t clock_hz, uint8_t sda_pin, uint8_t scl_pin);


// reads IC_STATUS and IC_TX_ABRT_SOURCE into status_out and abort_out
// useful for diagnosing NAK, arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint8_t i2c_num, uint32_t *status_out, uint32_t *abort_out);