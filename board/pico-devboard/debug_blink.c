/**
 * @file debug_blink.c
 * @brief Blinking LED functionality for debug purposes.
 */


#include "debug_blink.h"

void blink_loop(uint32_t blink_rate_ms) {
    while (1) {
        bm_gpio_toggle(25);
        bm_systick_delay_ms(blink_rate_ms);
    }
}

void blink_n_times(uint8_t blink_count) {
    for (int i = 0; i < blink_count; i++) {
        bm_gpio_put(25,true);
        bm_systick_delay_ms(500);
        bm_gpio_put(25,false);
        bm_systick_delay_ms(500);
    }
}