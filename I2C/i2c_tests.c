#include "i2c.h"
#include "../GPIO/gpio.h"
#include "../SysTick/systick.h"

#define I2C_ADDR 0x76 //address of BMP390 sensor, used for testing
#define I2C_ID_REG_ADDR 0x00 //register address of BMP390 ID register, used for testing
#define I2C_ID_REG_VAL 0x60 //expected value of BMP390 ID register, used for testing

int main() {
    bm_gpio_set_function(25, GPIO_FUNC_SIO);
    bm_gpio_set_direction(25, GPIO_DIR_OUTPUT);
    bm_gpio_put(25, false);
    bm_systick_init();

    //3 blinks before test
    for (int i=3; i > 0; i--) {
        bm_gpio_put(25, true);
        bm_systick_delay_ms(500);
        bm_gpio_put(25, false);
        bm_systick_delay_ms(500);
    }

    //test
    bool result;
    uint8_t i2c_num;
    result = bm_i2c_init(&i2c_num,I2C_MODE_STANDARD,0,1);
    
    //testing with write_read on ID register of BMP390 sensor at address 0x76, should read 0x60
    if (result) {
        uint8_t buf[1];
        result = bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);
        if (result && buf[0] == I2C_ID_REG_VAL) {
            //success
            bm_gpio_put(25, true);
            while(1) {
                //test passed, do nothing
            }
        } else {
            //read failed or incorrect value read
            while(1) {
                bm_gpio_toggle(25);
                bm_systick_delay_ms(500);
            }
        }
    } else {
        //initialization failed
        while(1) {
            bm_gpio_toggle(25);
            bm_systick_delay_ms(200);
        }
    }
}