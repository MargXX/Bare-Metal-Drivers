#include "i2c.h"
#include "i2c_reg.h"
#include "resets_reg.h"

// returns false on error, true on success

// initialize I2C peripheral in master mode — must be called before any other I2C functions
// configures GPIO pins, clock divider, and enables peripheral
bool bm_i2c_init(uint8_t *i2c_num, uint32_t clock_hz, uint8_t sda_pin, uint8_t scl_pin) {
    // validate pin pair and determine peripheral
    uint8_t peripheral = 255; //invalid peripheral value
    for (size_t i = 0; i < NUM_I2C_PAIRS; i++) {
        if (i2c_valid_pairs[i].sda == sda_pin && i2c_valid_pairs[i].scl == scl_pin) {
            peripheral = i2c_valid_pairs[i].peripheral;
            *i2c_num = peripheral; //output the peripheral number for use in other functions
            break;
        }
    }
    if (peripheral == 255) { return false; } //invalid pin pair
    // verify clock speed

    //dissasert reset  - RP2040 SPECIFIC
    RESETS->RESET &= ~i2c_resets_reset_mask[*i2c_num];
    // poll until reset is done
    uint32_t timeout = 0x0FFFFFFF;
    while (((RESETS->DONE & i2c_resets_reset_done_mask[*i2c_num])) == 0) {
        if (timeout == 0) {return false;}
        timeout--;
    }
    // disable I2C to allow configuration
    // disable DW_apb_i2c/set IC_ENABLE[0] = 0

    // configure GPIO pins
    bm_gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    bm_gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    // enable i2c, scl, and sda


    return false; //not implemented yet
}

// transmit len bytes from buf to device at 7-bit addr
// generates START, sends address + write bit, sends data, generates STOP
bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len) {
    //poll to see if data register empty
    //put data into IC_DAT data
    //write 1 to IC_DATA_CMD.STOP to get DW_apb_i2c to gernerate stop conditon
    return false; //not implemented yet
}

// receive len bytes from device at 7-bit addr into buf
// generates START, sends address + read bit, clocks in data, generates STOP
bool bm_i2c_read(uint8_t i2c_num, uint8_t addr, uint8_t *buf, size_t len) {
    return false; //not implemented yet
}

// write reg_addr, then read len bytes into buf without releasing the bus
// issues a repeated START between write and read phases
// this is the correct pattern for sensor register reads (BMP390, LSM9DS1)
bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
    return false; //not implemented yet
}

// returns true if the I2C bus or peripheral is currently busy
// check before initiating a transfer to avoid collisions
bool bm_i2c_is_busy(uint8_t i2c_num) {
    return false; //not implemented yet
}

// reads IC_STATUS and IC_TX_ABRT_SOURCE into status_out and abort_out
// useful for diagnosing NAK, arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint8_t i2c_num, uint32_t *status_out, uint32_t *abort_out) {
    return false; //not implemented yet
}

// disable and re-enable the I2C peripheral to recover from a hung bus
// use if SDA or SCL lines are stuck and normal transactions are not completing
bool bm_i2c_reset(uint8_t i2c_num) {
    return false; //not implemented yet
}

