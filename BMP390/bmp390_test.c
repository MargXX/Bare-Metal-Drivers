#include "bmp390.h"
#include "../SysTick/systick.h"
#include "../I2C/i2c.h"
#include "../UART/uart.h"
#include "../GPIO/gpio.h"
#include "../Debug/debug_print.h"
#include <stdio.h>




static void blink_loop(uint8_t blink_rate_ms) {
    while (1) {
        gpio_toggle(25);
        systick_delay_ms(blink_rate_ms);
    }
}

static void blink_n_times(uint8_t blink_count) {
    for (int i = 0; i < blink_count; i++) {
        bm_gpio_put(25,true);
        systick_delay_ms(500);
        bm_gpio_put(25,false);
        systick_delay_ms(500);
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
    if (!result) { blink_loop(3000); } //very slow blink on init failure
    uint8_t i2c_num;
    result = i2c_init(&i2c_num, I2C_MODE_STANDARD, 0, 1); // GP0/GP1
    if (!result) { blink_loop(3000); } //very slow blink on init failure

    // 3 blinks — signals start of test
    for (int i = 0; i < 3; i++) {
        gpio_toggle(25);
        systick_delay_ms(500);
    }

    // init bmp390
    bmp390_t dev;
    result = bmp390_init(&dev, i2c_num, BMP390_I2C_ADDR_DEFAULT );
    bm_debug_print_result(uart_num, result, "BMP390 init");

    // ID check
    uint8_t id;
    result = bmp390_read_chip_id(&dev, &id);
    if (!result || id != BMP390_CHIP_ID_VALUE) { 
        bm_debug_print_result(uart_num, false, "BMP390 ID check");
    } else {
        bm_debug_print_result(uart_num, true, "BMP390 ID check");
        bm_debug_print_labeled_hex8(uart_num, "BMP390 ID instead returned", id);
    }

    result = bmp390_soft_reset(&dev);
    bm_debug_print_result(uart_num, result, "BMP390 soft reset");

    //loops at botton to prevent hardware freakout after tests finish
    if (!result) { //fast blink on general failure
        blink_loop(100); 
    } else { //slow blink on success
        blink_loop(1000); 
    } 