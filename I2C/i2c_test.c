#include "i2c.h"
#include "../GPIO/gpio.h"
#include "../SysTick/systick.h"
#include "../UART/uart.h"
#include "i2c_reg.h"

#define I2C_ADDR        0x76
#define I2C_ID_REG_ADDR 0x00
#define I2C_ID_REG_VAL  0x60

// print a 32-bit value over UART as 8 hex digits — diagnostic only
static void uart_print_hex32(uint8_t uart_num, uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    char buf[10];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buf[2 + (7 - i)] = hex[(val >> (i * 4)) & 0xF];
    }
    buf[9] = '\n';
    for (int i = 0; i < 10; i++) {
        bm_uart_write_byte(uart_num, (uint8_t)buf[i]);
    }
}

int main() {
    // LED setup
    bm_gpio_set_function(25, GPIO_FUNC_SIO);
    bm_gpio_set_direction(25, GPIO_DIR_OUTPUT);
    bm_gpio_put(25, false);
    bm_systick_init();

    // 3 blinks — signals start of test
    for (int i = 0; i < 3; i++) {
        bm_gpio_put(25, true);
        bm_systick_delay_ms(500);
        bm_gpio_put(25, false);
        bm_systick_delay_ms(500);
    }

    // init UART for diagnostic output
    uint8_t uart_num;
    bm_uart_init(&uart_num, 115200, 8, 9); // GP8/GP9

    // init I2C
    uint8_t i2c_num;
    bool result = bm_i2c_init(&i2c_num, I2C_MODE_STANDARD, 0, 1); // GP0/GP1
    if (!result) {
        // fast blink — init failed
        while (1) {
            bm_gpio_toggle(25);
            bm_systick_delay_ms(1000);
        }
    }

    // attempt chip ID read
    uint8_t buf[1] = {0};
    result = bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);

    // print status and abort source regardless of result
    uint32_t status, abort_src, ic_con;
    bm_i2c_get_status(i2c_num, &status, &abort_src, &ic_con);
    uart_print_hex32(uart_num, status);
    uart_print_hex32(uart_num, abort_src);
    uart_print_hex32(uart_num, (uint32_t)buf[0]);
    uart_print_hex32(uart_num, ic_con);
    uint32_t expected_con = I2C_IC_CON_MASTER_MODE_Msk | (I2C_IC_CON_SPEED_STANDARD << I2C_IC_CON_SPEED_SHIFT) | I2C_IC_CON_IC_RESTART_EN_Msk | I2C_IC_CON_IC_SLAVE_DISABLE_Msk;
    uart_print_hex32(uart_num, expected_con);


    if (result && buf[0] == I2C_ID_REG_VAL) {
        // solid on — pass
        bm_gpio_put(25, true);
        while (1) {}
    } else {
        // slow blink — read failed or wrong value
        while (1) {
            bm_gpio_toggle(25);
            bm_systick_delay_ms(200);
        }
    }
}