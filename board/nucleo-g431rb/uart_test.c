#include "uart.h"
#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "gpio_platform.h"
#include "debug_blink.h"

/*
 * uart_test — HAL-backed UART on the Nucleo-G431RB.
 *
 * USART1 on PC4 (TX) / PC5 (RX) at 115200 8N1, read through an external
 * USB-serial adapter (adapter RX to PC4, adapter TX to PC5, shared GND).
 * PA2/PA3 are not used: they belong to the on-board ST-LINK virtual COM port
 * and did not reach the header on the test board.
 * Sends 0x55 once (alternating bits, easy to check on a logic analyzer),
 * then echoes any received byte between "Hello World" lines.
 *
 * LED solid on: init passed. Fast blink: init failed.
 */

#define UART_TX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 4)
#define UART_RX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 5)

int main() {
    bool result = bm_rcc_clock_init();
    result = result && bm_systick_init();

    bm_gpio_set_direction(DEBUG_LED_PIN, GPIO_DIR_OUTPUT);
    bm_gpio_put(DEBUG_LED_PIN, false);
    if (!result) { blink_loop(50); }

    //3 blinks before test
    blink_n_times(3);

    //test
    uint8_t uart_val;
    result = bm_uart_init(&uart_val, 115200, UART_TX_PIN, UART_RX_PIN);

    if (result) {
        bm_uart_write_byte(uart_val, 0x55);
        bm_gpio_put(DEBUG_LED_PIN, true);
        while(1) {
            bm_uart_write_str(uart_val, "Hello World - ");
            if (bm_uart_rx_ready(uart_val)) {
                uint8_t byte;
                if (bm_uart_read_byte(uart_val, &byte)) {
                    bm_uart_write_str(uart_val, "echo: ");
                    bm_uart_write_byte(uart_val, byte);
                }
            }
            bm_uart_write_str(uart_val, "\r\n");
            bm_systick_delay_ms(200);
        }
    } else {
        blink_loop(200);
    }
}
