#include "i2c.h"
#include "i2c_reg.h"
#include "resets_reg.h"


// functions returns false on error, true on success unless said otherwise in function comment

static bool bm_i2c_set_tar_address(uint8_t i2c_num, uint8_t addr);

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
    // verify clock speed and get high/low count values based on mode
    uint8_t speed_mode;
    uint32_t high_count;
    uint32_t low_count;
    if (clock_hz <= I2C_MODE_STANDARD) {
        speed_mode = I2C_IC_CON_SPEED_STANDARD;
        // high period lengths from datasheet
        high_count = (uint64_t)I2C_SS_MIN_HIGH_TIME * I2C_CLK / 1000000000UL;
        low_count = (uint64_t)I2C_SS_MIN_LOW_TIME * I2C_CLK / 1000000000UL;
    } else if (clock_hz <= I2C_MODE_FAST) {
        speed_mode = I2C_IC_CON_SPEED_FAST;
        high_count = (uint64_t)I2C_FS_MIN_HIGH_TIME * I2C_CLK / 1000000000UL;
        low_count = (uint64_t)I2C_FS_MIN_LOW_TIME * I2C_CLK / 1000000000UL;
    } else if (clock_hz <= I2C_MODE_FAST_PLUS) {
        speed_mode = I2C_IC_CON_SPEED_FAST_PLUS;
        high_count = (uint64_t)I2C_FS_MIN_PLUS_HIGH_TIME * I2C_CLK / 1000000000UL;
        low_count = (uint64_t)I2C_FS_MIN_PLUS_LOW_TIME * I2C_CLK / 1000000000UL;
    }else {
        return false; //invalid clock speed
    }


    //dissasert reset  - RP2040 SPECIFIC
    RESETS->RESET &= ~i2c_resets_reset_mask[*i2c_num];
    // poll until reset is done
    uint32_t timeout = TIMEOUT_CYCLES;
    while (((RESETS->DONE & i2c_resets_reset_done_mask[*i2c_num])) == 0) {
        if (timeout == 0) {return false;}
        timeout--;
    }
    // disable I2C to allow configuration:
    // clear IC_ENABLE[0]
    i2c_peripherals[*i2c_num]->IC_ENABLE &= ~I2C_IC_ENABLE_ENABLE_Msk;
    // poll until IC_ENABLE_STATUS[0] reads 0 to ensure the peripheral is fully disabled before continuing with configuration
    timeout = TIMEOUT_CYCLES;
    while (((i2c_peripherals[*i2c_num]->IC_ENABLE_STATUS & I2C_IC_ENABLE_STATUS_IC_EN_Msk)) != 0) {
        if (timeout == 0) {return false;}
        timeout--;
    }

    //enable master mode, disable slave mode, enable restart, and set speed in IC_CON
    i2c_peripherals[*i2c_num]->IC_CON = 
        (( I2C_IC_CON_MASTER_MODE_Msk) |
        ( speed_mode << I2C_IC_CON_SPEED_SHIFT ) |
        ( I2C_IC_CON_IC_RESTART_EN_Msk ) |
        ( I2C_IC_CON_IC_SLAVE_DISABLE_Msk ));


    // configure GPIO pins
    bm_gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    bm_gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    //set speed
    if (speed_mode == I2C_IC_CON_SPEED_STANDARD) {
        i2c_peripherals[*i2c_num]->IC_SS_SCL_HCNT = high_count & I2C_SS_SCL_HCNT_Msk;
        i2c_peripherals[*i2c_num]->IC_SS_SCL_LCNT = low_count & I2C_SS_SCL_LCNT_Msk;
    } else if (speed_mode == I2C_IC_CON_SPEED_FAST || speed_mode == I2C_IC_CON_SPEED_FAST_PLUS) {
        i2c_peripherals[*i2c_num]->IC_FS_SCL_HCNT = high_count & I2C_FS_SCL_HCNT_Msk;
        i2c_peripherals[*i2c_num]->IC_FS_SCL_LCNT = low_count & I2C_FS_SCL_LCNT_Msk;
    }
    // initilize IC_TAR with 0 since the target address will be set in each transaction function (write, read, write_read)
    i2c_peripherals[*i2c_num]->IC_TAR = 0; 

    // enable I2C by setting IC_ENABLE[0] to 1
    i2c_peripherals[*i2c_num]->IC_ENABLE |= I2C_IC_ENABLE_ENABLE_Msk;

    return true; // success
}

// transmit len bytes from buf to device at 7-bit addr
// generates START, sends address + write bit, sends data, generates STOP
bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len) {
    if (len == 0) { return true; } //nothing to write, likely error on users part, the function itself succeeded since there are no bytes to write, so return true
     // validate peripheral number
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    uint32_t total_timeout = TOTAL_TIMEOUT_CYCLES;
    uint32_t timeout = TIMEOUT_CYCLES;
    while (bm_i2c_is_busy(i2c_num)) { 
                if (timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                if (total_timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                total_timeout--;
                timeout--;
            }

    //address phase - generate start condition and send address + set to write mode
    if (!bm_i2c_set_tar_address(i2c_num, addr)) { return false; }
    //write mode is default

    //writing the bytes - poll to see if TX is ready, then put data into IC_DATA_CMD data field len times
    for (uint16_t i = 0; i < len; i++) {
        //poll and write byte when clear
        timeout = TIMEOUT_CYCLES;
        // check if TX FIFO is full
        while ((i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_TFNF_Msk) == 0) {  
            if (timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            if (total_timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            total_timeout--;
            timeout--;
        }
        //when empty write byte, this starts data transfer and generates clock pulses
        if (i == len - 1) {
            //if this is the last byte, set STOP bit to generate stop condition after transfer
            i2c_peripherals[i2c_num]->IC_DATA_CMD = (buf[i] & I2C_IC_DATA_CMD_DAT_Msk) | I2C_IC_DATA_CMD_STOP_Msk;
            
            //wait for transaction to complete before checking for errors in case of NAK on last byte
            volatile uint32_t delay = DELAY_CYCLE_COUNT;
            while (delay-- > 0) {} //gap to allow I2C_IC_STATUS_TFE to update
            timeout = TIMEOUT_CYCLES;
            while (bm_i2c_is_busy(i2c_num)) { 
                if (timeout == 0) {return false;}
                if (total_timeout == 0) {return false;}
                total_timeout--;
                timeout--;
            }
        } else {
            i2c_peripherals[i2c_num]->IC_DATA_CMD = buf[i] & I2C_IC_DATA_CMD_DAT_Msk;
        }
    }
    if ((i2c_peripherals[i2c_num]->IC_TX_ABRT_SOURCE & ~I2C_ICTX_ABRT_SOURCE_RESERVED_Msk) != 0) {
        // this condition indicates that the slave did not ACK a byte and the byte was lost
        return false;
    }

    return true; //success
}

// receive len bytes from device at 7-bit addr into buf
// generates START, sends address + read bit, clocks in data, generates STOP
bool bm_i2c_read(uint8_t i2c_num, uint8_t addr, uint8_t *buf, size_t len) {
    if (len == 0) { return true; } //nothing to read, likely error on users part, the function itself succeeded since there are no bytes to read, so return true
     // validate peripheral number
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    uint32_t total_timeout = TOTAL_TIMEOUT_CYCLES;
    uint32_t timeout = TIMEOUT_CYCLES;
    while (bm_i2c_is_busy(i2c_num)) { 
                if (total_timeout == 0) {return false;}
                total_timeout--;
            }

    //address phase - generate start condition and send address 
    if (!bm_i2c_set_tar_address(i2c_num, addr)) { return false; }

    //reading the bytes - poll to see if RX is ready, then read data from IC_DATA_CMD data field len times
    for (uint16_t i = 0; i < len; i++) {

        if (i == len - 1) {
            //if this is the last byte, set STOP bit to generate stop condition after transfer
            i2c_peripherals[i2c_num]->IC_DATA_CMD = I2C_IC_DATA_CMD_CMD_Msk | I2C_IC_DATA_CMD_STOP_Msk;
        } else {
            //request byte to read by writing to CMD
            i2c_peripherals[i2c_num]->IC_DATA_CMD = I2C_IC_DATA_CMD_CMD_Msk; 
        }
         
        //poll and read byte when ready
        timeout = TIMEOUT_CYCLES;
        while ((i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_RFNE_Msk) == 0) {  
            if (timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            if (total_timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            total_timeout--;
            timeout--;
        }
        //read the byte when ready
        buf[i] = i2c_peripherals[i2c_num]->IC_DATA_CMD & I2C_IC_DATA_CMD_DAT_Msk; //mask out the data bits and ignore the command bit
        if (i == len - 1) {
            //wait for transaction to complete before checking for errors in case of NAK on last byte
            timeout = TIMEOUT_CYCLES;
            while (bm_i2c_is_busy(i2c_num)) { 
                if (timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                if (total_timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                total_timeout--;
            }
        }
    }

    if ((i2c_peripherals[i2c_num]->IC_TX_ABRT_SOURCE & ~I2C_ICTX_ABRT_SOURCE_RESERVED_Msk) != 0) {
        // this condition indicates that the slave did not ACK the address byte and the read did not occur
        return false;
    }
    
    
    return true; //success
}

// write reg_addr, then read len bytes into buf without releasing the bus
// issues a repeated START between write and read phases
// pattern for sensor register reads (BMP390, LSM9DS1)
bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
     // validate peripheral number
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number
    if (len == 0) { return true; } //nothing to read, likely error on users part, the function itself succeeded since there are no bytes to read, so return true

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    uint32_t total_timeout = TOTAL_TIMEOUT_CYCLES;
    uint32_t timeout = TIMEOUT_CYCLES;
    while (bm_i2c_is_busy(i2c_num)) { 
                if (timeout == 0) {return false;}
                if (total_timeout == 0) {return false;}
                total_timeout--;
                timeout--;
            }

    //address phase - generate start condition and send address + set to write mode
    if (!bm_i2c_set_tar_address(i2c_num, addr)) { return false; }

    //poll and write byte when clear
    timeout = TIMEOUT_CYCLES;
        // check if TX FIFO is full
    while ((i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_TFNF_Msk) == 0) {  
        if (timeout == 0) {return false;}
        if (total_timeout == 0) {return false;}
        total_timeout--;
        timeout--;
    }
    //write the register address to specify the register we want to read from, this starts the transaction and generates clock pulses
    i2c_peripherals[i2c_num]->IC_DATA_CMD = reg_addr & I2C_IC_DATA_CMD_DAT_Msk;

    //wait for address byte to be sent and acknowledged before starting the read phase
    timeout = TIMEOUT_CYCLES;
    while ((i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_TFE_Msk) == 0) { 
        if (timeout == 0) {
            bm_i2c_reset(i2c_num);
            return false;
        }
        if (total_timeout == 0) {
            bm_i2c_reset(i2c_num);
            return false;
        }
        total_timeout--;
        timeout--;
    }
    

    //reading the bytes - poll to see if RX is ready, then read data from IC_DATA_CMD data field len times
    for (uint16_t i = 0; i < len; i++) {
        uint32_t data_cmd_mask = I2C_IC_DATA_CMD_CMD_Msk;
        if (i == 0) {
            data_cmd_mask |= I2C_IC_DATA_CMD_RESTART_Msk; //restart this as read
        }
        if (i == len - 1) {
            //if this is the last byte, set STOP bit to generate stop condition after transfer
            data_cmd_mask |= I2C_IC_DATA_CMD_STOP_Msk;
        }

        i2c_peripherals[i2c_num]->IC_DATA_CMD = data_cmd_mask;
         
        //poll and read byte when ready
        timeout = TIMEOUT_CYCLES;
        while ((i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_RFNE_Msk) == 0) {  
            if (timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            if (total_timeout == 0) {
                bm_i2c_reset(i2c_num);
                return false;
            }
            total_timeout--;
            timeout--;
        }
        //read the byte when ready
        buf[i] = i2c_peripherals[i2c_num]->IC_DATA_CMD & I2C_IC_DATA_CMD_DAT_Msk; //mask out the data bits and ignore the command bit
        if (i == len - 1) {
            //wait for transaction to complete before checking for errors in case of NAK on last byte
            timeout = TIMEOUT_CYCLES;
            while (bm_i2c_is_busy(i2c_num)) { 
                if (timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                if (total_timeout == 0) {
                    bm_i2c_reset(i2c_num);
                    return false;
                }
                total_timeout--;
            }
        }
    }

    // check for NACKS
    if ((i2c_peripherals[i2c_num]->IC_TX_ABRT_SOURCE & ~I2C_ICTX_ABRT_SOURCE_RESERVED_Msk) != 0) {
        // this condition indicates that the slave did not ACK a byte and the byte was lost
        return false;
    }

    return true; //success
}

// returns true if the I2C bus or peripheral is currently busy
// check before initiating a transfer to avoid collisions
bool bm_i2c_is_busy(uint8_t i2c_num) {
    return 
        (i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_TFE_Msk) == 0 || 
        (i2c_peripherals[i2c_num]->IC_STATUS & I2C_IC_STATUS_ACTIVITY_Msk) != 0 ;
}

// reads IC_STATUS and IC_TX_ABRT_SOURCE into status_out and abort_out
// useful for diagnosing NAK, arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint8_t i2c_num, uint32_t *status_out, uint32_t *abort_out) {
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number
    *status_out = i2c_peripherals[i2c_num]->IC_STATUS;
    *abort_out = i2c_peripherals[i2c_num]->IC_TX_ABRT_SOURCE;
    return true; //success
}

// disable and re-enable the I2C peripheral to recover from a hung bus
// use if SDA or SCL lines are stuck and normal transactions are not completing
bool bm_i2c_reset(uint8_t i2c_num) {
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number
    // disable I2C to allow configuration:
    // clear IC_ENABLE[0]
    i2c_peripherals[i2c_num]->IC_ENABLE &= ~I2C_IC_ENABLE_ENABLE_Msk;
    // poll until IC_ENABLE_STATUS[0] reads 0 to ensure the peripheral is fully disabled before continuing with configuration
    uint32_t timeout = TIMEOUT_CYCLES;
    while (((i2c_peripherals[i2c_num]->IC_ENABLE_STATUS & I2C_IC_ENABLE_STATUS_IC_EN_Msk)) != 0) {
        if (timeout == 0) { return false;}
        timeout--;
    }
    //read to clear abort source to clear the abort condition that caused the bus to hang
    (void)i2c_peripherals[i2c_num]->IC_TX_ABRT_SOURCE;

    //reenable
    i2c_peripherals[i2c_num]->IC_ENABLE |= I2C_IC_ENABLE_ENABLE_Msk;

    return true; //success
}


bool bm_i2c_set_tar_address(uint8_t i2c_num, uint8_t addr) {
    if (i2c_num >= NUM_I2C_PAIRS) { return false; }//invalid peripheral number
    // disable I2C to allow configuration:
    // clear IC_ENABLE[0]
    i2c_peripherals[i2c_num]->IC_ENABLE &= ~I2C_IC_ENABLE_ENABLE_Msk;
    // poll until IC_ENABLE_STATUS[0] reads 0 to ensure the peripheral is fully disabled before continuing with configuration
    uint32_t timeout = TIMEOUT_CYCLES;
    while (((i2c_peripherals[i2c_num]->IC_ENABLE_STATUS & I2C_IC_ENABLE_STATUS_IC_EN_Msk)) != 0) {
        if (timeout == 0) {return false;}
        timeout--;
    }

    //set address
    i2c_peripherals[i2c_num]->IC_TAR = (addr & I2C_IC_TAR_IC_TAR_7BIT_Msk); //write mode is default when setting address, so just write the address to IC_TAR to start the transaction

    //reenable
    i2c_peripherals[i2c_num]->IC_ENABLE |= I2C_IC_ENABLE_ENABLE_Msk;
    
    return true; //success
}


