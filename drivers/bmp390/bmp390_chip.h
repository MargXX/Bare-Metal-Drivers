/**
 * @file bmp390_chip.h
 * @brief Public, caller-facing constants for the BMP390 device driver
 *
 * For a memory-mapped peripheral, the "base address" is an internal hardware
 * fact and lives in the _reg.h. For an I2C device the closest analog is the
 * 7-bit bus address — but the CALLER selects it (it depends on how the SDO pin
 * is strapped on the board), then passes it into bm_bmp390_init. That makes it
 * exactly the kind of public constant this _chip.h exists for.
 *
 * Reference: BMP390 Datasheet, Section 5 (digital interfaces)
 */

#ifndef BMP390_CHIP_H
#define BMP390_CHIP_H

#include <stdint.h>

// 7-bit I2C device address, selected by the SDO pin strap:
//   SDO -> GND  : 0x76
//   SDO -> VDDIO: 0x77
#define BMP390_I2C_ADDR_SDO_LOW   ((uint8_t)0x76)
#define BMP390_I2C_ADDR_SDO_HIGH  ((uint8_t)0x77)

#define BMP390_CHIP_ID_VALUE 0x60

// Convenience default for the current board (SDO high). Change per board.
#define BMP390_I2C_ADDR_DEFAULT   BMP390_I2C_ADDR_SDO_HIGH

#endif /* BMP390_CHIP_H */