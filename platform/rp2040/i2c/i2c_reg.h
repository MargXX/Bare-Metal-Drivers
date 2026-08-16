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

#define I2C_CLK 125000000UL // 125MHz clock default

#define I2C_SS_MIN_HIGH_TIME        4000UL // ns
#define I2C_SS_MIN_LOW_TIME         4700UL // ns
#define I2C_FS_MIN_HIGH_TIME        600UL  // ns
#define I2C_FS_MIN_LOW_TIME         1300UL // ns
#define I2C_FS_MIN_PLUS_HIGH_TIME   260UL  // ns
#define I2C_FS_MIN_PLUS_LOW_TIME    500UL  // ns


#define I2C_DELAY_CYCLE_COUNT 8UL
#define I2C_TOTAL_TIMEOUT_CYCLES 0xFFFFFFFFUL
#define I2C_TIMEOUT_CYCLES 0x0FFFFFFFUL


typedef struct {  
    uint8_t sda;
    uint8_t scl;
    uint8_t peripheral;
} i2c_pin_pair;

//this is here so i2c.c is device portable
static const i2c_pin_pair i2c_valid_pairs[] = {
    { .sda =  0, .scl =  1, .peripheral = 0 },
    { .sda =  4, .scl =  5, .peripheral = 0 },
    { .sda =  8, .scl =  9, .peripheral = 0 },
    { .sda = 12, .scl = 13, .peripheral = 0 },
    { .sda = 16, .scl = 17, .peripheral = 0 },
    { .sda = 20, .scl = 21, .peripheral = 0 },
    { .sda =  2, .scl =  3, .peripheral = 1 },
    { .sda =  6, .scl =  7, .peripheral = 1 },
    { .sda = 10, .scl = 11, .peripheral = 1 },
    { .sda = 14, .scl = 15, .peripheral = 1 },
    { .sda = 18, .scl = 19, .peripheral = 1 },
    { .sda = 26, .scl = 27, .peripheral = 1 },
}; 

#define NUM_I2C_PAIRS (sizeof(i2c_valid_pairs) / sizeof(i2c_pin_pair))
#define NUM_I2C_PERIPHERALS 2

#define I2C0_BASE 0x40044000
#define I2C1_BASE 0x40048000


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

#define I2C_IC_CON_SPEED_STANDARD     0x1UL
#define I2C_IC_CON_SPEED_FAST         0b10UL
#define I2C_IC_CON_SPEED_FAST_PLUS    0b10UL
#define I2C_IC_CON_SPEED_HIGH         0b11UL

// Register masks
#define I2C_IC_CON_MASTER_MODE_Msk                  (1UL << 0)
#define I2C_IC_CON_SPEED_Msk                        (3UL << 1)
#define I2C_IC_CON_IC_10BITADDR_SLAVE_Msk           (1UL << 3)
#define I2C_IC_CON_IC_10BITADDR_MASTER_Msk          (1UL << 4)
#define I2C_IC_CON_IC_RESTART_EN_Msk                (1UL << 5)
#define I2C_IC_CON_IC_SLAVE_DISABLE_Msk             (1UL << 6)
#define I2C_IC_CON_STOP_DET_IFADDRESSED_Msk         (1UL << 7)
#define I2C_IC_CON_TX_EMPTY_CTRL_Msk                (1UL << 8)
#define I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_Msk        (1UL << 9)
#define I2C_IC_CON_STOP_DET_IF_MASTER_ACTIVE_Msk    (1UL << 10)

#define I2C_IC_TAR_IC_TAR_10BIT_Msk ((1UL << 10) - 1)
#define I2C_IC_TAR_IC_TAR_7BIT_Msk ((1UL << 7) - 1)

#define I2C_IC_SAR_IC_SAR_Msk ((1UL << 10) - 1)

#define I2C_IC_DATA_CMD_DAT_Msk             ((1UL << 8) - 1)
#define I2C_IC_DATA_CMD_CMD_Msk             (1UL << 8)
#define I2C_IC_DATA_CMD_STOP_Msk            (1UL << 9)
#define I2C_IC_DATA_CMD_RESTART_Msk         (1UL << 10)
#define I2C_IC_DATA_CMD_FIRST_DATA_BYTE_Msk (1UL << 11)

#define I2C_IC_ENABLE_ENABLE_Msk (1UL << 0)

#define I2C_IC_ENABLE_STATUS_IC_EN_Msk (1UL << 0)
#define I2C_IC_ENABLE_STATUS_SLV_DISABLED_WHILE_BUSY_Msk (1UL << 1)
#define I2C_IC_ENABLE_STATUS_SLV_RX_DATA_LOST_Msk (1UL << 2)

#define I2C_SS_SCL_HCNT_Msk ((1UL << 16) - 1)
#define I2C_SS_SCL_LCNT_Msk ((1UL << 16) - 1)
#define I2C_FS_SCL_HCNT_Msk ((1UL << 16) - 1)
#define I2C_FS_SCL_LCNT_Msk ((1UL << 16) - 1)

#define I2C_IC_CON_MASTER_MODE_SHIFT                  0
#define I2C_IC_CON_SPEED_SHIFT                        1
#define I2C_IC_CON_IC_10BITADDR_SLAVE_SHIFT           3
#define I2C_IC_CON_IC_10BITADDR_MASTER_SHIFT          4
#define I2C_IC_CON_IC_RESTART_EN_SHIFT                5
#define I2C_IC_CON_IC_SLAVE_DISABLE_SHIFT             6
#define I2C_IC_CON_STOP_DET_IFADDRESSED_SHIFT         7
#define I2C_IC_CON_TX_EMPTY_CTRL_SHIFT                8
#define I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_SHIFT        9
#define I2C_IC_CON_STOP_DET_IF_MASTER_ACTIVE_SHIFT    10

#define I2C_IC_STATUS_ACTIVITY_Msk (1UL << 0)
#define I2C_IC_STATUS_TFNF_Msk (1UL << 1)
#define I2C_IC_STATUS_TFE_Msk (1UL << 2)
#define I2C_IC_STATUS_RFNE_Msk (1UL << 3)
#define I2C_IC_STATUS_RFF_Msk (1UL << 4)
#define I2C_IC_STATUS_MST_ACTIVITY_Msk (1UL << 5)
#define I2C_IC_STATUS_SLV_ACTIVITY_Msk (1UL << 6)


#define I2C_ICTX_ABRT_SOURCE_7B_ADDR_NOACK_Msk (1UL << 0)
#define I2C_ICTX_ABRT_SOURCE_10BADDR1_NOACK_Msk (1UL << 1)
#define I2C_ICTX_ABRT_SOURCE_10BADDR2_NOACK_Msk (1UL << 2)
#define I2C_ICTX_ABRT_SOURCE_TXDATA_NOACK_Msk (1UL << 3)
#define I2C_ICTX_ABRT_SOURCE_GCALL_NOACK_Msk (1UL << 4)
#define I2C_ICTX_ABRT_SOURCE_GCALL_READ_NOACK_Msk (1UL << 5)
#define I2C_ICTX_ABRT_SOURCE_HS_ACKDET_Msk (1UL << 6)
#define I2C_ICTX_ABRT_SOURCE_SBYTE_ACKDET_Msk (1UL << 7)
#define I2C_ICTX_ABRT_SOURCE_HS_NORSTRT_Msk (1UL << 8)
#define I2C_ICTX_ABRT_SOURCE_SBYTE_NORSTRT_Msk (1UL << 9)
#define I2C_ICTX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Msk (1UL << 10)
#define I2C_ICTX_ABRT_SOURCE_MASTER_DIS_Msk (1UL << 11)
#define I2C_ICTX_ABRT_SOURCE_ARB_LOST_Msk (1UL << 12)
#define I2C_ICTX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_Msk (1UL << 13)
#define I2C_ICTX_ABRT_SOURCE_ABRT_SLV_ARBLOST_Msk (1UL << 14)
#define I2C_ICTX_ABRT_SOURCE_ABRT_SLVRD_INTX_Msk (1UL << 15)
#define I2C_ICTX_ABRT_SOURCE_ABRT_USER_ABRT_Msk (1UL << 16)
#define I2C_ICTX_ABRT_SOURCE_RESERVED_Msk (0b111111 << 17)
#define I2C_ICTX_ABRT_SOURCE_TX_FLUSH_CNT_Msk (0b111111111 << 23)

#endif /* I2C_REG_H */