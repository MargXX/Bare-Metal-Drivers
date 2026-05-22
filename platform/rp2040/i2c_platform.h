/**
 * @file i2c_platform.h
 * @brief Public platform constants for RP2040 I2C
 */


// I2C clock speed — update SCL_HCNT/LCNT in i2c.c if changing
// Standard mode (100kHz): I2C_MODE_STANDARD
// Fast mode (400kHz):     I2C_MODE_FAST
#define I2C_MODE_STANDARD   100000U
#define I2C_MODE_FAST       400000U

//enumerated values for IC_CON
#define I2C_IC_CON_MASTER_MODE_ENABLED                          0x1
#define I2C_IC_CON_MASTER_MODE_DISABLED                         0x0
#define I2C_IC_CON_SPEED_STANDARD                               0x1
#define I2C_IC_CON_SPEED_FAST                                   0x2
#define I2C_IC_CON_SPEED_HIGH                                   0x3
#define I2C_IC_CON_IC_10BITADDR_SLAVE_ADDR_7BITS                0x0
#define I2C_IC_CON_IC_10BITADDR_SLAVE_ADDR_10BITS               0x1
#define I2C_IC_CON_IC_10BITADDR_MASTER_ADDR_7BITS               0x0
#define I2C_IC_CON_IC_10BITADDR_MASTER_ADDR_10BITS              0x1
#define I2C_IC_CON_IC_RESTART_EN_ENABLED                        0x1
#define I2C_IC_CON_IC_RESTART_EN_DISABLED                       0x0
#define I2C_IC_CON_IC_SLAVE_DISABLE_SLAVE_ENABLED               0x1
#define I2C_IC_CON_IC_SLAVE_DISABLE_SLAVE_DISABLED              0x0
#define I2C_IC_CON_STOP_DET_IFADDRESSED_ENABLED                 0x1
#define I2C_IC_CON_STOP_DET_IFADDRESSED_DISABLED                0x0
#define I2C_IC_CON_TX_EMPTY_CTRL_ENABLED                        0x1
#define I2C_IC_CON_TX_EMPTY_CTRL_DISABLED                       0x0
#define I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_ENABLED                0x1
#define I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_DISABLED               0x0



#define I2C_IC_DATA_CMD_CMD_ENABLED                0x1
#define I2C_IC_DATA_CMD_CMD_DISABLED               0x0
#define I2C_IC_DATA_CMD_STOP_ENABLED               0x1
#define I2C_IC_DATA_CMD_STOP_DISABLED              0x0
#define I2C_IC_DATA_CMD_RESTART_ENABLED            0x1
#define I2C_IC_DATA_CMD_RESTART_DISABLED           0x0
#define I2C_IC_DATA_CMD_FIRST_DATA_BYTE_ENABLED    0x1
#define I2C_IC_DATA_CMD_FIRST_DATA_BYTE_DISABLED   0x0