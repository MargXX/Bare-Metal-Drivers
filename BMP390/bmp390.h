/**
 * @file bmp390.h
 * @brief BMP390 barometric pressure + temperature sensor driver (I2C)
 *
 * Bare-metal device driver layered on top of the bm_i2c_* transaction API.
 * No vendor HAL. Register details are isolated in bmp390_reg.h (included only
 * by bmp390.c); this header exposes only the public API and the types a caller
 * needs.
 *
 * Design notes:
 * The driver is host-agnostic: it talks to the sensor purely through the I2C
 * API and never touches RP2040 registers directly, so a future STM32 port only
 * needs an STM32 implementation of bm_i2c_*.
 *
 * Handle-based API: the caller allocates one bmp390_t per physical sensor and
 * passes &dev into every call. The handle carries the bus number, the device
 * address, and the factory calibration coefficients read at init. This keeps
 * the driver free of hidden global state and supports multiple BMP390s on one
 * bus (e.g. one at 0x76 and one at 0x77) with no driver changes.
 *
 * COMPENSATION: this build uses FLOATING-POINT compensation (Bosch float
 * formula). It is readable and easier to verify against the datasheet, but the
 * RP2040 (Cortex-M0+) has no FPU, so the math is emitted as software float.
 *   >>> PORT-LATER NOTE: a fixed-point (int64) version avoids software float
 *   and gives deterministic timing on the FPU-less M0+. 
 *
 * Reference: BMP390 Datasheet
 *   Compensation formula: Appendix (§9 / "output compensation", §3.11)
 *   Config registers: §4.3.17 (PWR_CTRL), §4.3.18 (OSR), §4.3.19 (ODR),
 *                     §4.3.21 (CONFIG)
 */

#ifndef BMP390_H
#define BMP390_H

#include <stdint.h>
#include <stdbool.h>
#include "bmp390_platform.h"

// All functions return false on error, true on success (matches the stack
// convention). Output values are written through caller-supplied pointers.


/* ------------------------------------------------------------------ */
/* Configuration enums                                                 */
/*                                                                     */
/* Enum VALUES mirror the datasheet field encodings so bmp390.c can    */
/* shift them straight into the register field. Verify each encoding   */
/* against the section noted — if you'd rather look them up yourself,  */
/* blank the values here and confirm from the datasheet.               */
/* ------------------------------------------------------------------ */

// Power mode — PWR_CTRL.mode (DS §4.3.17)
typedef enum {
    BMP390_MODE_SLEEP  = 0x00,
    BMP390_MODE_FORCED = 0x01,   // 0x02 is also "forced" per datasheet
    BMP390_MODE_NORMAL = 0x03,
} bmp390_mode_t;

// Oversampling — OSR.osr_p / OSR.osr_t (DS §4.3.18). Same encoding for both.
typedef enum {
    BMP390_OSR_X1  = 0x00,   // no oversampling
    BMP390_OSR_X2  = 0x01,
    BMP390_OSR_X4  = 0x02,
    BMP390_OSR_X8  = 0x03,
    BMP390_OSR_X16 = 0x04,
    BMP390_OSR_X32 = 0x05,
} bmp390_osr_t;

// Output data rate — ODR.odr_sel (DS §4.3.19/§4.3.20). Subsampling = 2^value;
// valid 0..17, higher value -> lower rate. Common picks below; add as needed.
typedef enum {
    BMP390_ODR_200_HZ  = 0x00,
    BMP390_ODR_100_HZ  = 0x01,
    BMP390_ODR_50_HZ   = 0x02,
    BMP390_ODR_25_HZ   = 0x03,
    BMP390_ODR_12P5_HZ = 0x04,
    // ... up to 0x11 (value saturates at 17)
} bmp390_odr_t;

// IIR filter coefficient — CONFIG.iir_filter (DS §4.3.21)
typedef enum {
    BMP390_IIR_COEF_0   = 0x00,   // filter off (bypass)
    BMP390_IIR_COEF_1   = 0x01,
    BMP390_IIR_COEF_3   = 0x02,
    BMP390_IIR_COEF_7   = 0x03,
    BMP390_IIR_COEF_15  = 0x04,
    BMP390_IIR_COEF_31  = 0x05,
    BMP390_IIR_COEF_63  = 0x06,
    BMP390_IIR_COEF_127 = 0x07,
} bmp390_iir_t;


/* ------------------------------------------------------------------ */
/* Public structs                                                      */
/* ------------------------------------------------------------------ */

// Sensor configuration the caller fills and passes to bm_bmp390_configure.
typedef struct {
    bool          press_en;   // enable pressure measurement
    bool          temp_en;    // enable temperature measurement (needed to
                              // compensate pressure — keep on)
    bmp390_mode_t mode;       // SLEEP / FORCED / NORMAL
    bmp390_osr_t  osr_p;      // pressure oversampling
    bmp390_osr_t  osr_t;      // temperature oversampling
    bmp390_odr_t  odr;        // output data rate (normal mode)
    bmp390_iir_t  iir;        // IIR filter coefficient
} bmp390_config_t;

// One compensated reading.
typedef struct {
    float pressure_pa;        // pressure in pascals
    float temperature_c;      // temperature in degrees Celsius
} bmp390_data_t;

// Quantized floating-point calibration coefficients (Bosch float formula).
// INTERNAL — populated by bm_bmp390_init, consumed by the compensation math
// in bmp390.c. Treat as opaque; do not modify from caller code.
typedef struct {
    float par_t1;
    float par_t2;
    float par_t3;
    float par_p1;
    float par_p2;
    float par_p3;
    float par_p4;
    float par_p5;
    float par_p6;
    float par_p7;
    float par_p8;
    float par_p9;
    float par_p10;
    float par_p11;
    float t_lin;              // linearized temperature, carried from the temp
                              // compensation into the pressure compensation
} bmp390_calib_t;

// Per-device handle. Allocate one per physical sensor (statically, no heap)
// and pass &dev into every call. Fields are managed by the driver.
typedef struct {
    uint8_t        i2c_num;       // which I2C peripheral (passed to bm_i2c_*)
    uint8_t        addr;          // 7-bit device address (see bmp390_platform.h)
    bmp390_calib_t calib;         // factory coefficients, read at init
    bool           initialized;   // set true once init succeeds
} bmp390_t;



// API                                                                
// ------------------------------------------------------------------ 

// Initialize a sensor handle: stores i2c_num/addr, verifies CHIP_ID, issues a
// soft reset, then reads and quantizes the calibration coefficients.
// Does NOT start measurements — call bm_bmp390_configure afterward.
// The I2C peripheral must already be initialized (bm_i2c_init) by the caller.
// Returns false if CHIP_ID is wrong or any I2C transaction fails.
bool bm_bmp390_init(bmp390_t *dev, uint8_t i2c_num, uint8_t dev_addr);

// Apply a configuration (enables, mode, oversampling, ODR, IIR filter).
// Writes PWR_CTRL, OSR, ODR, and CONFIG.
bool bm_bmp390_configure(bmp390_t *dev, const bmp390_config_t *cfg);

// Read one compensated sample. Intended for NORMAL mode: optionally checks the
// data-ready flag, burst-reads DATA_0..DATA_5, and applies compensation.
bool bm_bmp390_read(bmp390_t *dev, bmp390_data_t *out);

// Trigger a single FORCED-mode measurement, wait for data-ready (with a
// timeout via SysTick), then read and compensate. Use when sampling on demand
// rather than continuously.
bool bm_bmp390_read_forced(bmp390_t *dev, bmp390_data_t *out);

// Poll the data-ready state (STATUS.drdy_press / drdy_temp). Writes the result
// to *ready_out.
bool bm_bmp390_data_ready(bmp390_t *dev, bool *ready_out);

// Issue a soft reset (CMD = softreset) and wait for the device to come back.
bool bm_bmp390_soft_reset(bmp390_t *dev);

// Read CHIP_ID into *id_out (expected value defined in bmp390_reg.h). Useful
// as a standalone bus/sanity check.
bool bm_bmp390_read_chip_id(bmp390_t *dev, uint8_t *id_out);

// Read ERR_REG and STATUS into the provided pointers for diagnostics
// (fatal/cmd/conf errors, cmd-ready, data-ready). Pass NULL to skip either.
bool bm_bmp390_get_status(bmp390_t *dev, uint8_t *err_out, uint8_t *status_out);

// add later as needed: FIFO read path, interrupt (INT_CTRL) configuration,
// altitude conversion from pressure.

#endif /* BMP390_H */