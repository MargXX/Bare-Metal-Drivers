/**
 * @file i2c_reg.h
 * @brief I2C register map for the RP2040
 * includes pin pair definitions for peripheral selection and register struct definitions for direct access
 *
 * Reference: RP2040 Datasheet, Sections 4.3
 */
#ifndef I2C_REG_H
#define I2C_REG_H

#include <stdint.h>



#define I2C0_BASE 0x40044000
#define I2C1_BASE 0x40048000

#define RESETS_BASE 0x4000c000 

//register offsets from base
typedef struct {
    volatile uint32_t IC_CON;                // 0x00 Control Register
    volatile uint32_t IC_TAR;                // 0x04 Target Address Register
    volatile uint32_t IC_SAR;                // 0x08 Slave Address Register
    volatile uint32_t _reserved0[1];         // 0x0C reserved
    volatile uint32_t IC_DATA_CMD;           // 0x10 Data Buffer and Command Register
    volatile uint32_t IC_SS_SCL_HCNT;        // 0x14 Standard Speed SCL High Count
    volatile uint32_t IC_SS_SCL_LCNT;        // 0x18 Standard Speed SCL Low Count
    volatile uint32_t IC_FS_SCL_HCNT;        // 0x1C Fast Speed SCL High Count
    volatile uint32_t IC_FS_SCL_LCNT;        // 0x20 Fast Speed SCL Low Count
    volatile uint32_t _reserved1[2];         // 0x24-0x28 reserved
    volatile uint32_t IC_INTR_STAT;          // 0x2C Interrupt Status Register
    volatile uint32_t IC_INTR_MASK;          // 0x30 Interrupt Mask Register
    volatile uint32_t IC_RAW_INTR_STAT;      // 0x34 Raw Interrupt Status Register
    volatile uint32_t IC_RX_TL;              // 0x38 Receive FIFO Threshold Register
    volatile uint32_t IC_TX_TL;              // 0x3C Transmit FIFO Threshold Register
    volatile uint32_t IC_CLR_INTR;           // 0x40 Clear Combined Interrupt Register
    volatile uint32_t IC_CLR_RX_UNDER;       // 0x44 Clear RX_UNDER Interrupt
    volatile uint32_t IC_CLR_RX_OVER;        // 0x48 Clear RX_OVER Interrupt
    volatile uint32_t IC_CLR_TX_OVER;        // 0x4C Clear TX_OVER Interrupt
    volatile uint32_t IC_CLR_RD_REQ;         // 0x50 Clear RD_REQ Interrupt
    volatile uint32_t IC_CLR_TX_ABRT;        // 0x54 Clear TX_ABRT Interrupt
    volatile uint32_t IC_CLR_RX_DONE;        // 0x58 Clear RX_DONE Interrupt
    volatile uint32_t IC_CLR_ACTIVITY;       // 0x5C Clear ACTIVITY Interrupt
    volatile uint32_t IC_CLR_STOP_DET;       // 0x60 Clear STOP_DET Interrupt
    volatile uint32_t IC_CLR_START_DET;      // 0x64 Clear START_DET Interrupt
    volatile uint32_t IC_CLR_GEN_CALL;       // 0x68 Clear GEN_CALL Interrupt
    volatile uint32_t IC_ENABLE;             // 0x6C Enable Register
    volatile uint32_t IC_STATUS;             // 0x70 Status Register
    volatile uint32_t IC_TXFLR;              // 0x74 Transmit FIFO Level Register
    volatile uint32_t IC_RXFLR;              // 0x78 Receive FIFO Level Register
    volatile uint32_t IC_SDA_HOLD;           // 0x7C SDA Hold Time Register
    volatile uint32_t IC_TX_ABRT_SOURCE;     // 0x80 Transmit Abort Source Register
    volatile uint32_t IC_SLV_DATA_NACK_ONLY; // 0x84 Slave Data NACK Register
    volatile uint32_t IC_DMA_CR;             // 0x88 DMA Control Register
    volatile uint32_t IC_DMA_TDLR;           // 0x8C DMA Transmit Data Level Register
    volatile uint32_t IC_DMA_RDLR;           // 0x90 DMA Receive Data Level Register
    volatile uint32_t IC_SDA_SETUP;          // 0x94 SDA Setup Register
    volatile uint32_t IC_ACK_GENERAL_CALL;   // 0x98 ACK General Call Register
    volatile uint32_t IC_ENABLE_STATUS;      // 0x9C Enable Status Register
    volatile uint32_t IC_FS_SPKLEN;          // 0xA0 Spike Suppression Limit Register
    volatile uint32_t _reserved2[1];         // 0xA4 reserved
    volatile uint32_t IC_CLR_RESTART_DET;    // 0xA8 Clear RESTART_DET Interrupt
    volatile uint32_t _reserved3[0x12];      // 0xAC-0xF3 reserved
    volatile uint32_t IC_COMP_PARAM_1;       // 0xF4 Component Parameter Register 1
    volatile uint32_t IC_COMP_VERSION;       // 0xF8 Component Version Register
    volatile uint32_t IC_COMP_TYPE;          // 0xFC Component Type Register
} i2c_regs_t;

static volatile i2c_regs_t * const i2c_peripherals[]= {
    (volatile i2c_regs_t *)I2C0_BASE,
    (volatile i2c_regs_t *)I2C1_BASE,
};

typedef struct {
    volatile uint32_t RESET;    //RESETS_RESETS_OFFSET 0x0
    volatile uint32_t WDSEL;    //RESETS_WDSEL_OFFSET 0x4
    volatile uint32_t DONE;     //RESETS_RESET_DONE_OFFSET 0x8
} reset_regs_t;

#define RESETS ((volatile reset_regs_t *)RESETS_BASE)

// Register masks
#define RESETS_I2C0_Msk (1UL << 3)
#define RESETS_I2C1_Msk (1UL << 4)


static const uint32_t i2c_resets_reset_mask[] = {
    RESETS_I2C0_Msk,
    RESETS_I2C1_Msk,
};

static const uint32_t i2c_resets_reset_mask[] = {
    RESETS_I2C0_Msk,
    RESETS_I2C1_Msk,
};

#endif /* I2C_REG_H */