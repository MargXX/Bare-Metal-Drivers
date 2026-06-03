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