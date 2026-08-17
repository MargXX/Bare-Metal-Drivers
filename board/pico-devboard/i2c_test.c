#include "i2c.h"
#include "gpio.h"
#include "systick.h"
#include "uart.h"
#include "i2c_platform.h"
#include "gpio_platform.h"
#include "debug_print.h"
#include "debug_blink.h"

#define I2C_ADDR        0x77
#define I2C_ID_REG_ADDR 0x00
#define I2C_ID_REG_VAL  0x60


int main() {
    // LED setup
    bm_gpio_set_function(25, GPIO_FUNC_SIO);
    bm_gpio_set_direction(25, GPIO_DIR_OUTPUT);
    bm_gpio_put(25, false);
    bm_systick_init();

    // 3 blinks — signals start of test
    blink_n_times(3);

    // init UART for diagnostic output
    uint8_t uart_num;
    bm_uart_init(&uart_num, 115200, 8, 9); // GP8/GP9

    // init I2C
    uint8_t i2c_num;
    bool result = bm_i2c_init(&i2c_num, I2C_MODE_STANDARD, 0, 1); // GP0/GP1
    if (!result) {
        // fast blink — init failed
        blink_loop(50);
    }

    // attempt chip ID read
    uint8_t buf[1] = {0};
    result = bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);

    // print status and abort source regardless of result
    uint32_t status, abort_src;
    bm_i2c_get_status(i2c_num, &status, &abort_src);
    bm_debug_print_result(uart_num, result, "I2C write_read");
    bm_debug_print_hex32(uart_num, status);
    bm_debug_print_hex32(uart_num, abort_src);
    bm_debug_print_hex32(uart_num, (uint32_t)buf[0]);


    if (result && buf[0] == I2C_ID_REG_VAL) {
        // solid on — pass
        bm_gpio_put(25, true);
        while (1) {
            // bm_uart_write_str(uart_num,"Hello World - - - ");
            bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);
            
        }
    } else {
        // slow blink — read failed or wrong value
        blink_loop(1000);
    }
}