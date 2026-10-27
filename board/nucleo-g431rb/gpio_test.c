#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "gpio_platform.h"

/*
 * gpio_test — HAL-backed GPIO on the Nucleo-G431RB.
 *
 * PA5 (LD2) follows the B1 user button on PC13 (active low, external
 * pull-up on the board): LED off while the button is held, otherwise a
 * 10 Hz toggle. Exercises set_direction, put, get, and toggle.
 */

#define LED_PIN     BM_GPIO_PIN(BM_GPIO_PORT_A, 5)
#define BUTTON_PIN  BM_GPIO_PIN(BM_GPIO_PORT_C, 13)

int main() {
    bm_rcc_clock_init();
    bm_systick_init();

    bm_gpio_set_direction(LED_PIN, GPIO_DIR_OUTPUT);
    bm_gpio_set_direction(BUTTON_PIN, GPIO_DIR_INPUT);
    bm_gpio_put(LED_PIN, false);

    // bare metal programs never return from main
    // potential crash if we return from main, so just loop forever
    while(1) {
        bool released = true;
        bm_gpio_get(BUTTON_PIN, &released);
        if (released) {
            bm_gpio_toggle(LED_PIN);
        } else {
            bm_gpio_clear(LED_PIN);
        }
        bm_systick_delay_ms(50);
    }
}
