#include "bmp390.h"
#include "../SysTick/systick.h"
#include "../I2C/i2c.h"
#include "../UART/uart.h"
#include "../GPIO/gpio.h"
#include <stdio.h>


#define BMP390_CHIP_ID_VALUE 0x60

static void fail_blink(uint8_t blink_rate_ms) {
    while (1) {
        gpio_toggle(25);
        systick_delay_ms(blink_rate_ms);
    }
}

int main() {
    systick_init();
    // GPIO setup for LED and I2C/UART pins
    gpio_set_function(25, GPIO_FUNC_SIO);
    gpio_set_direction(25, GPIO_DIR_OUTPUT);
    //uart and i2c setup
    uint8_t uart_num;
    bool result;
    result = uart_init(&uart_num, 115200, 8, 9); // GP8/GP9
    if (!result) { fail_blink(3000); } //very slow blink on init failure
    uint8_t i2c_num;
    result = i2c_init(&i2c_num, I2C_MODE_STANDARD, 0, 1); // GP0/GP1
    if (!result) { fail_blink(3000); } //very slow blink on init failure

    // 3 blinks — signals start of test
    for (int i = 0; i < 3; i++) {
        gpio_toggle(25);
        systick_delay_ms(500);
    }

    // init bmp390
    bmp390_t dev;
    result = bmp390_init(&dev, i2c_num, BMP390_I2C_ADDR_DEFAULT );
    if (!result) { fail_blink(1000); } //slow blink on init failure

    // ID check
    uint8_t id;
    result = bmp390_read_chip_id(&dev, &id);
    if (!result || id != BMP390_CHIP_ID_VALUE) { fail_blink(200); } //fast blink on ID read 



}