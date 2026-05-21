#include "i2c.h"

// returns false on error, true on success

// initialize I2C peripheral in master mode — must be called before any other I2C functions
// configures GPIO pins, clock divider, and enables peripheral
bool bm_i2c_init(uint32_t clock_hz);

// transmit len bytes from buf to device at 7-bit addr
// generates START, sends address + write bit, sends data, generates STOP
bool bm_i2c_write(uint8_t addr, const uint8_t *buf, size_t len);

// receive len bytes from device at 7-bit addr into buf
// generates START, sends address + read bit, clocks in data, generates STOP
bool bm_i2c_read(uint8_t addr, uint8_t *buf, size_t len);

// write reg_addr, then read len bytes into buf without releasing the bus
// issues a repeated START between write and read phases
// this is the correct pattern for sensor register reads (BMP390, LSM9DS1)
bool bm_i2c_write_read(uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len);

// returns true if the I2C bus or peripheral is currently busy
// check before initiating a transfer to avoid collisions
bool bm_i2c_is_busy(void);

// reads IC_STATUS and IC_TX_ABRT_SOURCE into status_out and abort_out
// useful for diagnosing NAK, arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint32_t *status_out, uint32_t *abort_out);

// disable and re-enable the I2C peripheral to recover from a hung bus
// use if SDA or SCL lines are stuck and normal transactions are not completing
bool bm_i2c_reset(void);

