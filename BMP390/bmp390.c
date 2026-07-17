
#include "bmp390.h"
#include "bmp390_reg.h"
#include "../I2C/i2c.h"
#include "../SysTick/systick.h"

//powers of two for the floats. shortcut for readability in the calib parsing
static float power_of_two(uint16_t pow);

// compensate a raw temperature reading into a float in degrees C. Uses the bosch float formula from the datasheet. Updates the t_lin field in the calib struct for use in pressure compensation.
static float compensate_temperature(bmp390_calib_t *calib, uint32_t uncomp_temp);

// compensate a raw pressure reading into a float in pascals. Uses the bosch float formula from the datasheet. Uses the t_lin field in the calib struct, which must have been updated by a prior call to compensate_temperature.
static float compensate_pressure(bmp390_calib_t *calib, uint32_t uncomp_press);

// read and compensate a sample. Returns true if successful, false if any I2C transaction fails. Writes the compensated values to *out.
static bool read_and_compensate(bmp390_t *dev, bmp390_data_t *out);

// Initialize a sensor handle: stores i2c_num/addr, verifies CHIP_ID, issues a
// soft reset, then reads and quantizes the calibration coefficients.
// Does NOT start measurements — call bm_bmp390_configure afterward.
// The I2C peripheral must already be initialized (bm_i2c_init) by the caller.
// Returns false if CHIP_ID is wrong or any I2C transaction fails.
bool bm_bmp390_init(bmp390_t *dev, uint8_t i2c_num, uint8_t dev_addr) {
    //verify inputs
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    if (!dev) { return false; }
    //set fields for dev
    dev->initialized = false; // set this true when finished
    dev->configured = false; // set this true when finished
    //test chip ID to verify inputs
    dev->i2c_num = i2c_num;
    dev->addr = dev_addr;
    uint8_t id;
    if(!bm_bmp390_read_chip_id(dev, &id) || id != BMP390_CHIP_ID_VALUE) { return false;}
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
        .par_t1   =         (calib_arr[0] | ( calib_arr[1] << 8 )   )                       * power_of_two(8),     // 2^-8 
        .par_t2   =         (calib_arr[2] | ( calib_arr[3] << 8 )   )                       / power_of_two(30),    // 2^30 
        .par_t3   = (int8_t)(calib_arr[4]                           )                       / power_of_two(48),    // 2^48 signed
        .par_p1   =((int16_t)(calib_arr[5] | ( calib_arr[6] << 8 )  ) - power_of_two(14))  / power_of_two(20),    // 2^20 signed
        .par_p2   =((int16_t)(calib_arr[7] | ( calib_arr[8] << 8 )  ) - power_of_two(14))  / power_of_two(29),    // 2^29 signed
        .par_p3   = (int8_t)(calib_arr[9]                           )                       / power_of_two(32),    // 2^32 signed
        .par_p4   = (int8_t)(calib_arr[10]                          )                       / power_of_two(37),    // 2^37 signed
        .par_p5   =         (calib_arr[11] | ( calib_arr[12] << 8 ) )                       * power_of_two(3),     // 2^-3
        .par_p6   =         (calib_arr[13] | ( calib_arr[14] << 8 ) )                       / power_of_two(6),     // 2^6
        .par_p7   = (int8_t)(calib_arr[15]                          )                       / power_of_two(8),     // 2^8 signed
        .par_p8   = (int8_t)(calib_arr[16]                          )                       / power_of_two(15),    // 2^15 signed
        .par_p9   = (int16_t)(calib_arr[17] | ( calib_arr[18] << 8 ))                       / power_of_two(48),    // 2^48 signed
        .par_p10  = (int8_t)(calib_arr[19]                          )                       / power_of_two(48),    // 2^48 signed
        .par_p11  = (int8_t)(calib_arr[20]                          )                       / power_of_two(65),    // 2^65 signed
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
    if (dev == NULL) { return false; }
    if (cfg == NULL) { return false; }
    if (dev->initialized == false) { return false; }
    uint8_t buf[2];

    //write OSR
    uint8_t osr = 0;
    osr |= ((cfg->osr_p << BMP390_OSR_OSR_P_Pos) & BMP390_OSR_OSR_P_Msk);
    osr |= ((cfg->osr_t << BMP390_OSR_OSR_T_Pos) & BMP390_OSR_OSR_T_Msk);
    buf[0] = BMP390_REG_OSR;
    buf[1] = osr;
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        buf, 
        2
    )) { return false; }

    //write ODR
    uint8_t odr = 0;
    odr |= ((cfg->odr << BMP390_ODR_ODR_SEL_Pos) & BMP390_ODR_ODR_SEL_Msk);
    buf[0] = BMP390_REG_ODR;
    buf[1] = odr;
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        buf,
        2
    )) { return false; }

    //write CONFIG/IIR filter
    uint8_t config = 0;
    config |= ((cfg->iir << BMP390_CONFIG_IIR_FILTER_Pos) & BMP390_CONFIG_IIR_FILTER_Msk);
    buf[0] = BMP390_REG_CONFIG;
    buf[1] = config;
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        buf,
        2
    )) { return false; }

    //write PWR_CTRL
    uint8_t pwr_ctrl = 0;
    if (cfg->press_en) {
        pwr_ctrl |= BMP390_PWR_CTRL_PRESS_EN_Msk;
    }
    if (cfg->temp_en) {
        pwr_ctrl |= BMP390_PWR_CTRL_TEMP_EN_Msk;
    }
    pwr_ctrl |= ((cfg->mode << BMP390_PWR_CTRL_MODE_Pos) & BMP390_PWR_CTRL_MODE_Msk);
    buf[0] = BMP390_REG_PWR_CTRL;
    buf[1] = pwr_ctrl;
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        buf, 
        2
    )) { return false; }

    dev->cfg = *cfg; //store the last configuration applied for reference
    dev->configured = true; //set this true when finished

    return true; //success
}

// Read one compensated sample. Intended for NORMAL mode: optionally checks the
// data-ready flag, burst-reads DATA_0..DATA_5, and applies compensation.
bool bm_bmp390_read(bmp390_t *dev, bmp390_data_t *out) {
    if (dev == NULL) { return false; }
    if (out == NULL) { return false; }
    if (dev->initialized == false) { return false; }
    if (dev->configured == false) { return false; }

    if (!read_and_compensate(dev, out)) { return false; }

    return true; //success
}

// Trigger a single FORCED-mode measurement, wait for data-ready (with a
// timeout via SysTick), then read and compensate. Use when sampling on demand
// rather than continuously.
bool bm_bmp390_read_forced(bmp390_t *dev, bmp390_data_t *out) {
    if (dev == NULL) { return false; }
    if (out == NULL) { return false; }
    if (dev->initialized == false) { return false; }
    if (dev->configured == false) { return false; }

    //set power control to forced mode with the current enables
    uint8_t pwr_ctrl = 0;
    pwr_ctrl |= ((BMP390_MODE_FORCED << BMP390_PWR_CTRL_MODE_Pos) & BMP390_PWR_CTRL_MODE_Msk);
    if (dev->cfg.press_en) { pwr_ctrl |= BMP390_PWR_CTRL_PRESS_EN_Msk; }
    if (dev->cfg.temp_en) { pwr_ctrl |= BMP390_PWR_CTRL_TEMP_EN_Msk;}
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        (uint8_t[]){BMP390_REG_PWR_CTRL, pwr_ctrl}, 
        2
    )) { return false; }
    
    //wait for data ready with timeout
    uint32_t start_ms;
    bool ready = false;
    bm_systick_get_ms(&start_ms);
    while (!ready) {
        if (bm_systick_timeout_elapsed(start_ms, BMP390_TIMEOUT_MS)) { return false; }
        bm_bmp390_data_ready(dev, &ready);
    }

    if (!read_and_compensate(dev, out)) { return false; }

    return true; //success
}

// Poll the data-ready state (STATUS.drdy_press / drdy_temp). Writes the result
// to *ready_out. Returns true when both temperature and pressure are ready. Can only return true if both temperature and pressure are enabled.
bool bm_bmp390_data_ready(bmp390_t *dev, bool *ready_out) {
    if (dev == NULL) { return false; }
    if (ready_out == NULL) { return false; }
    if (dev->initialized == false) { return false; }
    if (dev->configured == false) { return false; }

    uint8_t statusbuf[1] = {0};
    if (!bm_i2c_write_read(
        dev->i2c_num, 
        dev->addr, 
        BMP390_REG_STATUS, 
        statusbuf, 
        1
    )) {return false;}
    ready_out[0] = (statusbuf[0] & (BMP390_STATUS_DRDY_PRESS_Msk | BMP390_STATUS_DRDY_TEMP_Msk)) == (BMP390_STATUS_DRDY_PRESS_Msk | BMP390_STATUS_DRDY_TEMP_Msk);
    
    return true; //success
}

// Issue a soft reset (CMD = softreset) and wait for the device to come back.
bool bm_bmp390_soft_reset(bmp390_t *dev) {
    if (dev == NULL) { return false; }

    uint8_t buf[2] = {BMP390_REG_CMD, BMP390_CMD_SOFTRESET};
    //write soft reset
    if (!bm_i2c_write(
        dev->i2c_num, 
        dev->addr, 
        buf, 
        2
    )) { return false; }
    //poll to wait until done
    uint32_t timeout = BMP390_TIMEOUT_CYCLES;
    uint8_t statusbuf[1] = {0};
    //make a delay to allow the device to process the reset before polling for status
    bm_systick_delay_ms(2);
    while ((statusbuf[0] & BMP390_STATUS_CMD_RDY_Msk) == 0) {
        if (timeout == 0) { return false; }
        timeout--;
        bm_i2c_write_read(dev->i2c_num, dev->addr, BMP390_REG_STATUS, statusbuf, 1);
    }
    return true; //success
}

// Read CHIP_ID into *id_out (expected value defined in bmp390_reg.h). Useful
// as a standalone bus/sanity check.
bool bm_bmp390_read_chip_id(bmp390_t *dev, uint8_t *id_out) {
    if (dev == NULL) { return false; }
    if (id_out == NULL) { return false; }

    uint8_t id_arr[1];
    if (!bm_i2c_write_read(dev->i2c_num, dev->addr, BMP390_REG_CHIP_ID, id_arr, 1 )) { return false; }
    *id_out = id_arr[0];
    return true; //success
}

// Read ERR_REG and STATUS into the provided pointers for diagnostics
// (fatal/cmd/conf errors, cmd-ready, data-ready). Pass NULL to skip either but will return false if both are skipped.
bool bm_bmp390_get_status(bmp390_t *dev, uint8_t *err_out, uint8_t *status_out) {
    if (dev == NULL) { return false; }
    if (!dev->initialized) { return false; }
    if (err_out == NULL && status_out == NULL) { return false; } //likely user error, but the function itself succeeded since there are no outputs to write to, so return true
    if (status_out != NULL && !bm_i2c_write_read(dev->i2c_num, dev->addr, BMP390_REG_STATUS, status_out, 1 )) { return false; }
    if (err_out != NULL && !bm_i2c_write_read(dev->i2c_num, dev->addr, BMP390_REG_ERR_REG, err_out, 1 )) { return false; }
    return true; //success
}

// Fill a bmp390_config_t with a reasonable default configuration. Caller can
// then modify fields before passing to bm_bmp390_configure. Returns false if the pointer is NULL.
bool bm_bmp390_default_config(bmp390_config_t *cfg){
    if (cfg == NULL) { return false; }
    cfg->press_en = true;
    cfg->temp_en = true;
    cfg->mode = BMP390_MODE_NORMAL;
    cfg->osr_p = BMP390_OSR_X8;
    cfg->osr_t = BMP390_OSR_X1;
    cfg->odr = BMP390_ODR_25_HZ;
    cfg->iir = BMP390_IIR_COEF_0;
    return true; //success
}


static float power_of_two(uint16_t pow){
    return ldexpf(1.0f, pow); // returns 1.0 * 2^pow, handles positive 
}

//code sourced from Bosch BMP390 datasheet, section 8.5
static float compensate_temperature(bmp390_calib_t *calib, uint32_t uncomp_temp) {

    float partial_data1 = (float)(uncomp_temp - calib->par_t1);
    float partial_data2 = (float)(partial_data1 * calib->par_t2);


    //update t_lin as needed for pressure compensation
    calib->t_lin = partial_data2 + (partial_data1 * partial_data1) * calib->par_t3;

    return calib->t_lin;
}

//code sourced from Bosch BMP390 datasheet, section 8.6
static float compensate_pressure(bmp390_calib_t *calib, uint32_t uncomp_press) {
    //var to store compensated pressure
    float comp_press;
    //calculate partial data
    float partial_data1 = calib->par_p6 * calib->t_lin;
    float partial_data2 = calib->par_p7 * (calib->t_lin * calib->t_lin);
    float partial_data3 = calib->par_p8 * (calib->t_lin * calib->t_lin * calib->t_lin);
    float partial_out1 = calib->par_p5 + partial_data1 + partial_data2 + partial_data3;
    
    partial_data1 = calib->par_p2 * calib->t_lin;
    partial_data2 = calib->par_p3 * (calib->t_lin * calib->t_lin);
    partial_data3 = calib->par_p4 * (calib->t_lin * calib->t_lin * calib->t_lin);
    float partial_out2 = (float)uncomp_press * (calib->par_p1 + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = (float)uncomp_press * (float)uncomp_press;
    partial_data2 = calib->par_p9 + calib->par_p10 * calib->t_lin;
    partial_data3 = partial_data1 * partial_data2;
    float partial_data4 = partial_data3 + ((float)uncomp_press * (float)uncomp_press * (float)uncomp_press) * calib->par_p11;
    
    comp_press = partial_out1 + partial_out2 + partial_data4;

    return comp_press;
}

// read and compensate a sample. Returns true if successful, false if any I2C transaction fails. Writes the compensated values to *out.
static bool read_and_compensate(bmp390_t *dev, bmp390_data_t *out) {
    if (dev == NULL) { return false; }
    if (out == NULL) { return false; }
    if (dev->initialized == false) { return false; }
    if (dev->configured == false) { return false; }

    uint8_t buf[BMP390_DATA_LEN]; 

    //read raw data
    if (!bm_i2c_write_read(
        dev->i2c_num, 
        dev->addr, 
        BMP390_REG_DATA_0, 
        buf, 
        BMP390_DATA_LEN
    )) { return false; }

    //convert lSB->MSB to 24-bit signed integers
    uint32_t press_raw = 0; //pressure data
    for (int i = 0; i<BMP390_PRESS_DATA_LEN; i++) {
        press_raw |= ((uint32_t)buf[i] << (8*i));
    }
    uint32_t temp_raw = 0; //temperature data
    for (int i = 0; i<BMP390_TEMP_DATA_LEN; i++) {
        temp_raw |= ((uint32_t)buf[i + BMP390_PRESS_DATA_LEN] << (8*i));
    }

    //temperature compensation
    out->temperature_c = compensate_temperature(&dev->calib, temp_raw);

    //pressure compensation
    out->pressure_pa = compensate_pressure(&dev->calib, press_raw);

    return true; //success
}