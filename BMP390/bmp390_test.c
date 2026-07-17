#include "bmp390.h"
#include "../SysTick/systick.h"
#include "../I2C/i2c.h"
#include "../UART/uart.h"
#include "../GPIO/gpio.h"
#include "../Debug/debug_print.h"
#include "../Debug/debug_blink.h"
#include "bmp390_reg.h"

int main() {
    bm_systick_init();

    // GPIO setup for LED and I2C/UART pins
    bm_gpio_set_function(25, GPIO_FUNC_SIO);
    bm_gpio_set_direction(25, GPIO_DIR_OUTPUT);

    //uart and i2c setup
    uint8_t uart_num;
    bool result;
    result = bm_uart_init(&uart_num, 115200, 8, 9); // GP8/GP9
    if (!result) { blink_loop(100); } //very slow blink on init failure
    uint8_t i2c_num;
    result = bm_i2c_init(&i2c_num, I2C_MODE_STANDARD, 0, 1); // GP0/GP1
    if (!result) { blink_loop(100); } //very slow blink on init failure

    // 3 blinks — signals start of test
    blink_n_times(3);

    //TESTS BELOW
    //---------------------------------------------

    bmp390_t dev;
    //init
    result = bm_bmp390_init(&dev, i2c_num, BMP390_I2C_ADDR_DEFAULT );
    bm_debug_print_result(uart_num, result, "BMP390 init");

    // ID check
    uint8_t id;
    result = bm_bmp390_read_chip_id(&dev, &id);
    if (!result || id != BMP390_CHIP_ID_VALUE) { 
        bm_debug_print_result(uart_num, false, "BMP390 ID check");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ID instead returned", id);
    } else {
        bm_debug_print_result(uart_num, true, "BMP390 ID check");
        
    }

    //soft reset
    result = bm_bmp390_soft_reset(&dev);
    bm_debug_print_result(uart_num, result, "BMP390 soft reset");

    //get_status
    uint8_t err, status;
    result = bm_bmp390_get_status(&dev, &err, &status);
    bm_debug_print_result(uart_num, result, "BMP390 get status");

    //check calibration data, read twice to verify consistency
    uint8_t calib1[BMP390_CALIB_DATA_LEN];
    uint8_t calib2[BMP390_CALIB_DATA_LEN];
    
    bool read1_ok = bm_i2c_write_read(
            i2c_num, 
            dev.addr, 
            BMP390_REG_CALIB_DATA, 
            calib1, 
            BMP390_CALIB_DATA_LEN
        );
    if (!result) {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }
    bm_systick_delay_ms(2); //wait a bit before reading again
    bool read2_ok = bm_i2c_write_read(
            i2c_num, 
            dev.addr, 
            BMP390_REG_CALIB_DATA, 
            calib2, 
            BMP390_CALIB_DATA_LEN
        );
    result = read1_ok && read2_ok;
    bm_debug_print_result(uart_num, result, "BMP390 calib read twice");
    if (result) {
        //check if the two reads match byte for byte
        for (uint8_t i = 0; i < BMP390_CALIB_DATA_LEN; i++) {
            if (calib1[i] != calib2[i]) {
                result = false;
            }
        }
    } else {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    } 
    bm_debug_print_result(uart_num, result, "BMP390 calib read consistency");


    //configure with default settings
    result = bm_bmp390_default_config(&dev.cfg);
    bm_debug_print_result(uart_num, result, "BMP390 default config");
    result = bm_bmp390_configure(&dev, &dev.cfg);
    bm_debug_print_result(uart_num, result, "BMP390 configure with default config");
    if (!result) {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }

    //data_ready
    bool ready;
    result = bm_bmp390_data_ready(&dev, &ready);
    bm_debug_print_result(uart_num, result, "BMP390 data ready");
    if (result) {
        bm_uart_write_str(uart_num, "BMP390 data ready: ");
        bm_debug_print_bool(uart_num, ready);
    } else {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }

    //read normal
    while (!ready) {
        bm_bmp390_data_ready(&dev, &ready);
    }
    bmp390_data_t data;
    result = bm_bmp390_read(&dev, &data);
    bm_debug_print_result(uart_num, result, "BMP390 read normal");
    if (result) {
        bm_uart_write_str(uart_num, "BMP390 pressure (Pa)");
        bm_debug_print_dec32(uart_num, (uint32_t)data.pressure_pa);
        bm_uart_write_str(uart_num, "BMP390 temperature (C)");
        bm_debug_print_dec32(uart_num, (uint32_t)data.temperature_c);
    } else {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }

    //read forced
    result = bm_bmp390_read_forced(&dev, &data);
    bm_debug_print_result(uart_num, result, "BMP390 read forced");
    if (result) {
        bm_uart_write_str(uart_num, "BMP390 pressure (Pa)");
        bm_debug_print_dec32(uart_num, (uint32_t)data.pressure_pa);
        bm_uart_write_str(uart_num, "BMP390 temperature (C)");
        bm_debug_print_dec32(uart_num, (uint32_t)data.temperature_c);
    } else {
        result = bm_bmp390_get_status(&dev, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }

    bm_uart_write_str(uart_num, "BMP390 test complete. Blinking LED to signal end of test.");


    blink_loop(1000);// everything done, blink to signal end of test

}

//TODO: test null guards