#include "bmp390.h"
#include "systick.h"
#include "i2c.h"
#include "uart.h"
#include "gpio.h"
#include "debug_print.h"
#include "debug_blink.h"
#include "bmp390_reg.h"


static void print_status_check(bmp390_t *dev, uint8_t uart_num) {
    uint8_t err = 0xA5; 
    uint8_t status = 0xA5;
    bool result = bm_bmp390_get_status(dev, &err, &status);       
    bm_debug_print_result(uart_num, result, "BMP390 get status");                        
    if (result) {
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err);  
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    } 
}

//testing this works on the pico, this feels very silly
static float power_of_two(uint16_t pow){
    return ldexpf(1.0f, pow); // returns 1.0 * 2^pow, handles positive 
}


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
    // blink_n_times(3);


    //TESTS BELOW
    //---------------------------------------------
    bm_uart_write_str(uart_num, "\n\n");

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

    
    result = bm_bmp390_get_status(&dev, NULL, &status); //should be true
    bm_debug_print_result(uart_num, result, "BMP390 get status null status only");
    result = bm_bmp390_get_status(&dev, &err, NULL); //should be true
    bm_debug_print_result(uart_num, result, "BMP390 get status null err only");

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
    if (!read1_ok) { print_status_check(&dev, uart_num); }
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
            // bm_debug_print_hex32(uart_num, (uint32_t)(calib1[i]) );
        }
    } else { print_status_check(&dev, uart_num); } 
    bm_debug_print_result(uart_num, result, "BMP390 calib read consistency");


    //configure with default settings
    result = bm_bmp390_default_config(&dev.cfg);
    bm_debug_print_result(uart_num, result, "BMP390 default config");
    result = bm_bmp390_configure(&dev, &dev.cfg);
    bm_debug_print_result(uart_num, result, "BMP390 configure with default config");
    if (!result) { print_status_check(&dev, uart_num); } 

    //data_ready
    bool ready;
    result = bm_bmp390_data_ready(&dev, &ready);
    bm_debug_print_result(uart_num, result, "BMP390 data ready");
    if (result) {
        bm_uart_write_str(uart_num, "BMP390 data ready: ");
        bm_debug_print_bool(uart_num, ready);
    } else { print_status_check(&dev, uart_num); } 

    //read normal
    uint16_t timeout_ms = 500;
    uint32_t start_time;
    bmp390_data_t data;
    bm_systick_get_ms(&start_time);
    while (!ready && !bm_systick_timeout_elapsed(start_time,timeout_ms)) {
        bm_bmp390_data_ready(&dev, &ready);
    }
    if (!ready) {
        result = false;
        bm_debug_print_result(uart_num, result ,"BMP390 read data_ready timeout");
    } else {
        result = bm_bmp390_read(&dev, &data);
        bm_debug_print_result(uart_num, result, "BMP390 read normal");
        if (result) {
            bm_uart_write_str(uart_num, "BMP390 pressure (Pa)");
            bm_debug_print_dec32(uart_num, (uint32_t)data.pressure_pa);
            bm_uart_write_str(uart_num, "BMP390 temperature (C)");
            bm_debug_print_dec32_signed(uart_num, (int32_t)data.temperature_c);
        } else { print_status_check(&dev, uart_num); } 
    }
    
    

    //read forced
    dev.cfg.mode = BMP390_MODE_SLEEP;
    result = bm_bmp390_configure(&dev,&(dev.cfg));
    bm_debug_print_result(uart_num, result, "BMP390 read forced configure");
    result = bm_bmp390_read_forced(&dev, &data);
    bm_debug_print_result(uart_num, result, "BMP390 read forced");
    if (result) {
        bm_uart_write_str(uart_num, "BMP390 pressure (Pa)");
        bm_debug_print_dec32(uart_num, (uint32_t)data.pressure_pa);
        bm_uart_write_str(uart_num, "BMP390 temperature (C)");
        bm_debug_print_dec32_signed(uart_num, (int32_t)data.temperature_c);
    } else { print_status_check(&dev, uart_num); } 
    //cleanup
    result =  bm_bmp390_default_config(&(dev.cfg));
    result &= bm_bmp390_configure(&dev,&(dev.cfg));
    bm_debug_print_result(uart_num, result, "BMP390 read forced cleanup");

    //null guard tests
    bmp390_t dev4 = dev; //shallow copy
    bm_uart_write_str(uart_num, "BMP390 null guard tests---------------\r\n");
    //init
    result = bm_bmp390_init(NULL, i2c_num, BMP390_I2C_ADDR_DEFAULT);
    result |= bm_bmp390_init(&dev4, 255, BMP390_I2C_ADDR_DEFAULT);
    bm_debug_print_result(uart_num, !result, "BMP390 init null");
    if (result) { print_status_check(&dev4, uart_num); } 
    //configure
    result = bm_bmp390_configure(NULL, &dev4.cfg);
    result |= bm_bmp390_configure(&dev4, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 configure null");
    if (result) { print_status_check(&dev4, uart_num); } 
    //read
    result = bm_bmp390_read(NULL, &data);
    result |= bm_bmp390_read(&dev4, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 read null");
    if (result) {
        result = bm_bmp390_get_status(&dev4, &err, &status);
        bm_debug_print_result(uart_num, result, "BMP390 get status");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ERR_REG", err );
        bm_debug_print_labeled_hex8(uart_num, "BMP390 STATUS", status);
    }  
    //read forced
    result = bm_bmp390_read_forced(NULL, &data);
    result |= bm_bmp390_read_forced(&dev4, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 read forced null");
    if (result) { print_status_check(&dev4, uart_num); } 
    //data ready
    result = bm_bmp390_data_ready(NULL, &ready);
    result |= bm_bmp390_data_ready(&dev4, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 data ready null");
    if (result) { print_status_check(&dev4, uart_num); } 
    //soft reset
    result = bm_bmp390_soft_reset(NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 soft reset null");
    if (result) { print_status_check(&dev4, uart_num); } 
    //read chip id
    result = bm_bmp390_read_chip_id(NULL, &id);
    result |= bm_bmp390_read_chip_id(&dev4, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 read chip id null");
    if (result) { print_status_check(&dev4, uart_num); } 

    //get status
    result = bm_bmp390_get_status(NULL, &err, &status);
    result |= bm_bmp390_get_status(&dev4, NULL, NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 get status null false");
    //default config
    result = bm_bmp390_default_config(NULL);
    bm_debug_print_result(uart_num, !result, "BMP390 default config null");
    if (result) { print_status_check(&dev4, uart_num); }


    //init guard tests
    bm_uart_write_str(uart_num, "BMP390 init guard tests---------------\r\n");
    bmp390_t dev2 = dev; //shallow copy dev to dev2 to test init guard
    dev2.initialized = false; //force unconfigured state
    dev2.configured = true; //force configured state to isolate init guard
    //init already tested when init is off
    //config
    result = bm_bmp390_configure(&dev2, &dev2.cfg);
    bm_debug_print_result(uart_num, !result, "BMP390 configure init guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //read
    result = bm_bmp390_read(&dev2, &data);
    bm_debug_print_result(uart_num, !result, "BMP390 read init guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //read forced
    result = bm_bmp390_read_forced(&dev2, &data);
    bm_debug_print_result(uart_num, !result, "BMP390 read forced init guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //data ready
    result = bm_bmp390_data_ready(&dev2, &ready);
    bm_debug_print_result(uart_num, !result, "BMP390 data ready init guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //chip ID and soft reset in init do dont need init guard
    //get status
    result = bm_bmp390_get_status(&dev2, &err, &status);
    bm_debug_print_result(uart_num, !result, "BMP390 get status init guard");
    if (result) { print_status_check(&dev, uart_num); } 

    //configure guard tests
    bm_uart_write_str(uart_num, "BMP390 configure guard tests---------------\r\n");
    dev2.configured = false; //force unconfigured state
    dev2.initialized = true; //force initialized state to isolate configure guard
    //just the reads and data ready need configure guard
    //read
    result = bm_bmp390_read(&dev2, &data);
    bm_debug_print_result(uart_num, !result, "BMP390 read configure guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //read forced
    result = bm_bmp390_read_forced(&dev2, &data);
    bm_debug_print_result(uart_num, !result, "BMP390 read forced configure guard");
    if (result) { print_status_check(&dev, uart_num); } 
    //data ready
    result = bm_bmp390_data_ready(&dev2, &ready);
    bm_debug_print_result(uart_num, !result, "BMP390 data ready configure guard");
    if (result) { print_status_check(&dev, uart_num); }


    
    //soft reset test
    bmp390_t dev3;
    result = bm_bmp390_init(&dev3, i2c_num, BMP390_I2C_ADDR_DEFAULT);
    bm_bmp390_default_config(&(dev3.cfg));
    dev3.cfg.mode = BMP390_MODE_FORCED; //nonstandard mode
    result &= bm_bmp390_configure(&dev3, &dev3.cfg);
    result &= bm_bmp390_soft_reset(&dev3);
    //check all settings at default
    uint8_t pwr_ctrl = 0xA5;
    uint8_t osr = 0x5A;
    result &= bm_i2c_write_read(i2c_num, dev3.addr, BMP390_REG_PWR_CTRL, &pwr_ctrl, 1);
    result &= bm_i2c_write_read(i2c_num, dev3.addr, BMP390_REG_OSR, &osr, 1);
    bm_debug_print_result(uart_num, result, "BMP390 soft reset check");
    result = osr == 0x00; //datasheet incorrect from testing
    bm_debug_print_result(uart_num, result, "BMP390 soft reset check osr");
    bm_debug_print_hex8(uart_num, osr);
    result = pwr_ctrl == 0x00; //default from datsheet
    bm_debug_print_result(uart_num, result, "BMP390 soft reset check pwr_ctrl");
    bm_debug_print_hex8(uart_num, pwr_ctrl);
    if (!result) { 
        print_status_check(&dev3, uart_num); 
    }

    //test power of two function on pico, just in case...
    result = power_of_two(0) == 1;
    result &= power_of_two(1) == 2;
    result &= power_of_two(3) == 8;
    result &= power_of_two(5) == 32;
    result &= power_of_two(8) == 256;
    bm_debug_print_result(uart_num,result,"BMP390 Power of Two test");

    bm_uart_write_str(uart_num, "BMP390 test complete. Blinking LED to signal end of test. \r\n");


    blink_loop(1000);// everything done, blink to signal end of test

}
