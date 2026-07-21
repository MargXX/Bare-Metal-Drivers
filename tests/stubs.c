

//fake i2c and systick calls for bmp390 go here

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


bool bm_systick_delay_ms(uint32_t ms) {
    (void)ms;
    return true; 
}


bool bm_systick_get_ms(uint32_t *ms_out) {
    *ms_out = 0;
    return true;
}

//real function returns true if timeout_ms has elapsed
bool bm_systick_timeout_elapsed(uint32_t start_ms, uint32_t timeout_ms) {
    (void)start_ms;
    (void)timeout_ms;
    return true; 
}

//return false if i2c not initialized and true if it is
bool bm_i2c_is_valid(uint8_t i2c_num){
    (void)i2c_num;
    return true;
}


bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len){
    (void)i2c_num;
    (void)addr;
    (void)buf;
    (void)len;
    return true; //no other user feedback needed
}


bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
    (void)i2c_num;
    (void)addr;
    (void)reg_addr;
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = len;
    }
    return true;
}


