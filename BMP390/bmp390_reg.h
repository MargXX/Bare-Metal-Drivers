/**
 * @file bmp390_reg.h
 * @brief BMP390 register map (chip-level, bus-agnostic)
 *
 * Registers live on the device and are reached over I2C by
 * sending an 8-bit register address (bm_i2c_write_read / bm_i2c_write). So this
 * file is a flat list of register-address #defines plus bit-field positions and
 * masks — no volatile struct overlay, no base pointer.
 *
 * This file describes the CHIP, not the RP2040, so it lives in the BMP390/
 * device folder (NOT platform/rp2040/). On a future STM32 port this file does
 * not change — only the I2C layer underneath it does.
 *
 * Reference: BMP390 Datasheet, Section 4 (memory map + register description)
 *            Memory map: Table 25 (§4.2)
 *            Trimming coefficients: Table 24 (§3.11.1)
 */

#ifndef BMP390_REG_H
#define BMP390_REG_H

#include <stdint.h>



// Identity / expected values                                         
#define BMP390_CHIP_ID_VALUE          // expected CHIP_ID readback — DS §4.3.1


// Register addresses (DS §4.2, Table 25)                             

#define BMP390_REG_CHIP_ID            // DS §4.3.1
#define BMP390_REG_REV_ID             // DS §4.3.2
#define BMP390_REG_ERR_REG            // DS §4.3.3
#define BMP390_REG_STATUS             // DS §4.3.4

#define BMP390_REG_DATA_0             // press_7_0    — DS §4.3.5
#define BMP390_REG_DATA_1             // press_15_8   — DS §4.3.5
#define BMP390_REG_DATA_2             // press_23_16  — DS §4.3.5
#define BMP390_REG_DATA_3             // temp_7_0     — DS §4.3.6
#define BMP390_REG_DATA_4             // temp_15_8    — DS §4.3.6
#define BMP390_REG_DATA_5             // temp_23_16   — DS §4.3.6

#define BMP390_REG_SENSORTIME_0       // DS §4.3.7
#define BMP390_REG_SENSORTIME_1       // DS §4.3.7
#define BMP390_REG_SENSORTIME_2       // DS §4.3.7

#define BMP390_REG_EVENT              // DS §4.3.8
#define BMP390_REG_INT_STATUS         // DS §4.3.9

#define BMP390_REG_PWR_CTRL           // DS §4.3.17
#define BMP390_REG_OSR                // DS §4.3.18
#define BMP390_REG_ODR                // DS §4.3.19
#define BMP390_REG_CONFIG             // DS §4.3.21

#define BMP390_REG_CMD                // DS §4.3.23



// ERR_REG fields (DS §4.3.3) — single-bit flags                      

#define BMP390_ERR_FATAL_Msk          // fatal_err
#define BMP390_ERR_CMD_Msk            // cmd_err  (cleared on read)
#define BMP390_ERR_CONF_Msk           // conf_err (cleared on read)



// STATUS fields (DS §4.3.4) — single-bit flags                       

#define BMP390_STATUS_CMD_RDY_Msk     // cmd_rdy
#define BMP390_STATUS_DRDY_PRESS_Msk  // drdy_press
#define BMP390_STATUS_DRDY_TEMP_Msk   // drdy_temp



// EVENT fields (DS §4.3.8) — single-bit flags                       

#define BMP390_EVENT_POR_DETECTED_Msk // por_detected
#define BMP390_EVENT_ITF_ACT_PT_Msk   // itf_act_pt



// INT_STATUS fields (DS §4.3.9) — single-bit flags                  
// ------------------------------------------------------------------ 

#define BMP390_INT_STATUS_FWM_Msk     // fwm_int
#define BMP390_INT_STATUS_FFULL_Msk   // ffull_int
#define BMP390_INT_STATUS_DRDY_Msk    // drdy



// PWR_CTRL fields (DS §4.3.17)                                      
// ------------------------------------------------------------------

#define BMP390_PWR_CTRL_PRESS_EN_Msk  // press_en (single bit)
#define BMP390_PWR_CTRL_TEMP_EN_Msk   // temp_en  (single bit)
#define BMP390_PWR_CTRL_MODE_Pos      // mode field LSB position
#define BMP390_PWR_CTRL_MODE_Msk      // mode field mask (2 bits)



// OSR fields (DS §4.3.18)                                           
// ------------------------------------------------------------------

#define BMP390_OSR_OSR_P_Pos          // osr_p field LSB position
#define BMP390_OSR_OSR_P_Msk          // osr_p field mask (3 bits)
#define BMP390_OSR_OSR_T_Pos          // osr_t field LSB position
#define BMP390_OSR_OSR_T_Msk          // osr_t field mask (3 bits)



// ODR fields (DS §4.3.19)                                            
// ------------------------------------------------------------------
#define BMP390_ODR_ODR_SEL_Pos        // odr_sel field LSB position
#define BMP390_ODR_ODR_SEL_Msk        // odr_sel field mask (5 bits)



// CONFIG fields (DS §4.3.21)                                        
// ------------------------------------------------------------------
#define BMP390_CONFIG_SHORT_IN_Msk    // short_in (single bit)
#define BMP390_CONFIG_IIR_FILTER_Pos  // iir_filter field LSB positio
#define BMP390_CONFIG_IIR_FILTER_Msk  // iir_filter field mask (3 bits



// CMD register (DS §4.3.23)                                          
// ------------------------------------------------------------------ 

#define BMP390_CMD_NOP                // 0x00
#define BMP390_CMD_FIFO_FLUSH         // clears FIFO
#define BMP390_CMD_SOFTRESET          // triggers power-on-reset


/* ------------------------------------------------------------------ */
/* Calibration / trimming coefficients (DS §3.11.1, Table 24)          */
/*                                                                     */
/* Read this whole block in one burst at init, then parse into the     */
/* quantized float coefficients (see bmp390_calib_t in bmp390.h).      */
/*                                                                     */
/* Layout (little-endian, low byte first):                             */
/*   NVM_PAR_T1   0x31..0x32   u16                                      */
/*   NVM_PAR_T2   0x33..0x34   u16                                      */
/*   NVM_PAR_T3   0x35         s8                                       */
/*   NVM_PAR_P1   0x36..0x37   s16                                      */
/*   NVM_PAR_P2   0x38..0x39   s16                                      */
/*   NVM_PAR_P3   0x3A         s8                                       */
/*   NVM_PAR_P4   0x3B         s8                                       */
/*   NVM_PAR_P5   0x3C..0x3D   u16                                      */
/*   NVM_PAR_P6   0x3E..0x3F   u16                                      */
/*   NVM_PAR_P7   0x40         s8                                       */
/*   NVM_PAR_P8   0x41         s8                                       */
/*   NVM_PAR_P9   0x42..0x43   s16                                      */
/*   NVM_PAR_P10  0x44         s8                                       */
/*   NVM_PAR_P11  0x45         s8                                       */
/* ------------------------------------------------------------------ */

#define BMP390_REG_CALIB_DATA         // start address of the calib block (DS Table 24)
#define BMP390_CALIB_DATA_LEN         // number of bytes to burst-read (count the rows above)


/* ================================================================== */
/* OPTIONAL / ADVANCED — fill these in only when you add interrupts,   */
/* the FIFO, or the I2C watchdog. Not needed for a basic forced/normal */
/* mode pressure+temperature read.                                     */
/* ================================================================== */

#define BMP390_REG_INT_CTRL           // DS §4.3.15
#define BMP390_REG_IF_CONF            // DS §4.3.16
#define BMP390_REG_FIFO_LENGTH_0      // DS §4.3.10
#define BMP390_REG_FIFO_LENGTH_1      // DS §4.3.10
#define BMP390_REG_FIFO_DATA          // DS §4.3.11
#define BMP390_REG_FIFO_WTM_0         // DS §4.3.12
#define BMP390_REG_FIFO_WTM_1         // DS §4.3.12
#define BMP390_REG_FIFO_CONFIG_1      // DS §4.3.13
#define BMP390_REG_FIFO_CONFIG_2      // DS §4.3.14

/* INT_CTRL fields (DS §4.3.15) */
#define BMP390_INT_CTRL_INT_OD_Msk        // int_od
#define BMP390_INT_CTRL_INT_LEVEL_Msk     // int_level
#define BMP390_INT_CTRL_INT_LATCH_Msk     // int_latch
#define BMP390_INT_CTRL_FWTM_EN_Msk       // fwtm_en
#define BMP390_INT_CTRL_FFULL_EN_Msk      // ffull_en
#define BMP390_INT_CTRL_INT_DS_Msk        // int_ds
#define BMP390_INT_CTRL_DRDY_EN_Msk       // drdy_en

/* IF_CONF fields (DS §4.3.16) */
#define BMP390_IF_CONF_SPI3_Msk           // spi3
#define BMP390_IF_CONF_I2C_WDT_EN_Msk     // i2c_wdt_en
#define BMP390_IF_CONF_I2C_WDT_SEL_Msk    // i2c_wdt_sel


#endif /* BMP390_REG_H */