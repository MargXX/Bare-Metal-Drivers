#include "uart.h"
#include "gpio.h"
#include "systick.h"
#include "gpio_platform.h"


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
    uint8_t uart_val;
    result = bm_uart_init(&uart_val,115200,8,9);

    if (result) {
        bm_uart_write_byte(uart_val,0x55);
        bm_gpio_put(25, true);
        while(1) {
            bm_uart_write_str(uart_val,"Hello World - ");
            // bm_systick_delay_ms(200);
        }
    } else {
        while(1) {
            // bm_uart_write_byte(uart_val,0x0F);
            bm_gpio_toggle(25);
            bm_systick_delay_ms(200);
            
        }
    }
    
}