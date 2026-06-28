
#include "bmp390.h"
#include "bmp390_reg.h"
#include "../I2C/i2c.h"
#include "../SysTick/systick.h"

//powers of two for the floats. shortcut for readability in the calib parsing
static float power_of_two(uint8_t pow);

// Initialize a sensor handle: stores i2c_num/addr, verifies CHIP_ID, issues a
// soft reset, then reads and quantizes the calibration coefficients.
// Does NOT start measurements — call bm_bmp390_configure afterward.
// The I2C peripheral must already be initialized (bm_i2c_init) by the caller.
// Returns false if CHIP_ID is wrong or any I2C transaction fails.
bool bm_bmp390_init(bmp390_t *dev, uint8_t i2c_num, uint8_t dev_addr) {
    //verify inputs
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    if (!dev) { return false; }
    //setit feilds for dev
    dev->initialized = false; // set this true when finished
    //test chip ID to verify inputs
    uint8_t id_arr[1];
    if (!bm_i2c_write_read(i2c_num, dev_addr, BMP390_REG_CHIP_ID, id_arr, 1) || id_arr[0] != BMP390_CHIP_ID_VALUE) { return false; }
    dev->i2c_num = i2c_num;
    dev->addr = dev_addr;
    //soft reset the device to ensure it's in a known state before reading calibration data
    if (!bm_bmp390_soft_reset(dev)) { return false; }
    //grab calib
    uint8_t calib_arr[BMP390_CALIB_DATA_LEN];
    if (!bm_i2c_write_read(
            i2c_num, 
            dev_addr, 
            BMP390_REG_CALIB_DATA, 
            calib_arr, 
            BMP390_CALIB_DATA_LEN
        )) { return false; }
    // translate into floats with powers from datasheet
    bmp390_calib_t calib_data = {
        .par_t1   =         (calib_arr[0] + ( calib_arr[1] << 8 )   )    * power_of_two(8),     // 2^-8 
        .par_t2   =         (calib_arr[2] + ( calib_arr[3] << 8 )   )    / power_of_two(30),    // 2^30 
        .par_t3   = ((int8_t)calib_arr[4]                           )    / power_of_two(48),    // 2^48 signed
        .par_p1   = ((int8_t)calib_arr[5] + ( calib_arr[6] << 8 )   )    / power_of_two(20),    // 2^20 signed
        .par_p2   = ((int8_t)calib_arr[7] + ( calib_arr[8] << 8 )   )    / power_of_two(29),    // 2^29 signed
        .par_p3   = ((int8_t)calib_arr[9]                           )    / power_of_two(32),    // 2^32 signed
        .par_p4   = ((int8_t)calib_arr[10]                          )    / power_of_two(37),    // 2^37 signed
        .par_p5   =         (calib_arr[11] + ( calib_arr[12] << 8 ) )    * power_of_two(3),     // 2^-3
        .par_p6   =         (calib_arr[13] + ( calib_arr[14] << 8 ) )    / power_of_two(6),     // 2^6
        .par_p7   = ((int8_t)calib_arr[15]                          )    / power_of_two(8),     // 2^8 signed
        .par_p8   = ((int8_t)calib_arr[16]                          )    / power_of_two(15),    // 2^15 signed
        .par_p9   = ((int8_t)calib_arr[17] + ( calib_arr[18] << 8 )  )    / power_of_two(48),    // 2^48 signed
        .par_p10  = ((int8_t)calib_arr[19]                          )    / power_of_two(48),    // 2^48 signed
        .par_p11  = ((int8_t)calib_arr[20]                          )    / power_of_two(65),    // 2^65 signed
        .t_lin = 0.0f,
    };
    dev->calib = calib_data;
    
    //declare initialization working
    dev->initialized = true;
    return true; //success
}

// Apply a configuration (enables, mode, oversampling, ODR, IIR filter).
// Writes PWR_CTRL, OSR, ODR, and CONFIG.
bool bm_bmp390_configure(bmp390_t *dev, const bmp390_config_t *cfg) {
    return false; //not yet implemented
}

// Read one compensated sample. Intended for NORMAL mode: optionally checks the
// data-ready flag, burst-reads DATA_0..DATA_5, and applies compensation.
bool bm_bmp390_read(bmp390_t *dev, bmp390_data_t *out) {
    return false; //not yet implemented
}

// Trigger a single FORCED-mode measurement, wait for data-ready (with a
// timeout via SysTick), then read and compensate. Use when sampling on demand
// rather than continuously.
bool bm_bmp390_read_forced(bmp390_t *dev, bmp390_data_t *out) {
    return false; //not yet implemented
}

// Poll the data-ready state (STATUS.drdy_press / drdy_temp). Writes the result
// to *ready_out.
bool bm_bmp390_data_ready(bmp390_t *dev, bool *ready_out) {
    return false; //not yet implemented
}

// Issue a soft reset (CMD = softreset) and wait for the device to come back.
bool bm_bmp390_soft_reset(bmp390_t *dev) {
    return false; //not yet implemented
}

// Read CHIP_ID into *id_out (expected value defined in bmp390_reg.h). Useful
// as a standalone bus/sanity check.
bool bm_bmp390_read_chip_id(bmp390_t *dev, uint8_t *id_out) {
    return false; //not yet implemented
}

// Read ERR_REG and STATUS into the provided pointers for diagnostics
// (fatal/cmd/conf errors, cmd-ready, data-ready). Pass NULL to skip either.
bool bm_bmp390_get_status(bmp390_t *dev, uint8_t *err_out, uint8_t *status_out) {
    return false; //not yet implemented
}


float power_of_two(uint8_t pow){
    return ldexpf(1.0f, pow); // returns 1.0 * 2^pow, handles positive and negative powers
}