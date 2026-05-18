#include "gpio.h"
#include "../SysTick/systick.h"


int main() {
    bm_gpio_set_function(25, GPIO_FUNC_SIO);
    bm_gpio_set_direction(25, GPIO_DIR_OUTPUT);
    bm_gpio_put(25, false);
    bm_systick_init();

    // bare metal programs never return from main
    // potential crash if we return from main, so just loop forever
    while(1) {
        bm_gpio_toggle(25);
        bm_systick_delay_ms(500);
    }
}