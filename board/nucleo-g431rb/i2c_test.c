#include "i2c.h"
#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "uart.h"
#include "i2c_platform.h"
#include "gpio_platform.h"
#include "debug_print.h"
#include "debug_blink.h"

/*
 * i2c_test — HAL-backed I2C on the Nucleo-G431RB.
 *
 * Reads the BMP390 CHIP_ID over I2C1 on PB9 (SDA, D14) / PB8 (SCL, D15)
 * and reports over USART1, PC4 (TX) / PC5 (RX), at 115200 8N1.
 *
 * LED solid on: pass. Slow blink: read failed or wrong ID. Fast blink: init failed.
 */

#define I2C_ADDR        0x77
#define I2C_ID_REG_ADDR 0x00
#define I2C_ID_REG_VAL  0x60

#define UART_TX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 4)
#define UART_RX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 5)
#define I2C_SDA_PIN BM_GPIO_PIN(BM_GPIO_PORT_B, 9)
#define I2C_SCL_PIN BM_GPIO_PIN(BM_GPIO_PORT_B, 8)


int main() {
    bool result = bm_rcc_clock_init();
    result = result && bm_systick_init();

    // LED setup
    bm_gpio_set_direction(DEBUG_LED_PIN, GPIO_DIR_OUTPUT);
    bm_gpio_put(DEBUG_LED_PIN, false);
    if (!result) { blink_loop(50); }

    // 3 blinks — signals start of test
    blink_n_times(3);

    // init UART for diagnostic output
    uint8_t uart_num;
    bm_uart_init(&uart_num, 115200, UART_TX_PIN, UART_RX_PIN);

    // init I2C
    uint8_t i2c_num;
    result = bm_i2c_init(&i2c_num, I2C_MODE_STANDARD, I2C_SDA_PIN, I2C_SCL_PIN);
    bm_debug_print_result(uart_num, result, "I2C init");
    if (!result) {
        // fast blink — init failed
        blink_loop(50);
    }

    // attempt chip ID read
    uint8_t buf[1] = {0};
    result = bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);

    // print status and HAL error code regardless of result
    uint32_t status, error;
    bm_i2c_get_status(i2c_num, &status, &error);
    bm_debug_print_result(uart_num, result, "I2C write_read");
    bm_debug_print_hex32(uart_num, status);
    bm_debug_print_hex32(uart_num, error);
    bm_debug_print_hex32(uart_num, (uint32_t)buf[0]);

    // NAK path: nothing answers at 0x10, so this must fail and recover
    uint8_t nak_buf[1];
    bool nak_result = bm_i2c_write_read(i2c_num, 0x10, I2C_ID_REG_ADDR, nak_buf, 1);
    bm_i2c_get_status(i2c_num, &status, &error);
    bm_debug_print_result(uart_num, !nak_result, "I2C NAK on absent address");
    bm_debug_print_hex32(uart_num, error);
    bm_debug_print_result(uart_num, bm_i2c_is_valid(i2c_num), "I2C valid after NAK recovery");


    if (result && buf[0] == I2C_ID_REG_VAL) {
        // solid on — pass
        bm_gpio_put(DEBUG_LED_PIN, true);
        while (1) {
            bm_i2c_write_read(i2c_num, I2C_ADDR, I2C_ID_REG_ADDR, buf, 1);
        }
    } else {
        // slow blink — read failed or wrong value
        blink_loop(1000);
    }
}
