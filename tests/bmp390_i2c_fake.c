

//fake i2c calls for bmp390 go here

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../I2C/i2c.h"
#include "../BMP390/bmp390_platform.h"
#include "../BMP390/bmp390_reg.h"

#define REG_COUNT 256

extern uint32_t clock_ms;
uint8_t regs[REG_COUNT];
uint8_t fail_after_n_calls; // fail when 0
bool fake_reset_signals_ready;
bool fake_reset_restores_defaults;

//returns true when i2c_num == 0
bool bm_i2c_is_valid(uint8_t i2c_num){
    return i2c_num == 0;
}


bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len){
    (void)i2c_num;
    clock_ms++;
    if (addr != BMP390_I2C_ADDR_DEFAULT) { return false; }
    if (len == 0) { return true; }
    if (buf[0] + len > REG_COUNT) { return false; }
    if (fail_after_n_calls == 0) { 
        fail_after_n_calls = 0xFF;
        return false; 
    } else if (fail_after_n_calls != 0xFF ){
        fail_after_n_calls--;
    }
    //if reset change status
    if (len == 2 && buf[0] == BMP390_REG_CMD && buf[1] == BMP390_CMD_SOFTRESET) {
        if (fake_reset_signals_ready) {
            regs[BMP390_REG_STATUS] = BMP390_STATUS_CMD_RDY_Msk;
        }
        if (fake_reset_restores_defaults) {
            regs[BMP390_REG_PWR_CTRL] = BMP390_POWER_ON_PWR_CTRL_DEFAULT;
            regs[BMP390_REG_OSR] = BMP390_POWER_ON_OSR_DEFAULT;
        }
    }
    for (size_t i=0; i<(len - 1); i++) {//first byte is reg addr
        regs[buf[0] + i] = buf[i+1];
    }
    return true; //no other user feedback needed
}

//no edits to regs since this is effectively a read function
bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
    (void)i2c_num;
    clock_ms++;
    if (addr != BMP390_I2C_ADDR_DEFAULT) { return false; }
    if (reg_addr + len > REG_COUNT) { return false; }
    if (fail_after_n_calls == 0) {
        fail_after_n_calls = 0xFF; 
        return false; 
    } else if (fail_after_n_calls != 0xFF ){
        fail_after_n_calls--;
    }

    for (size_t i = 0; i < len; i++) {
            buf[i] = regs[reg_addr + i];
    }
    return true;
}


