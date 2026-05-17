#include "gpio.h"

#define TIMER_LENGTH 0x000FFFFF

int main() {

    // bare metal programs never return from main
    // potential crash if we return from main, so just loop forever
    bm_gpio_enable(25);
    bm_gpio_set_direction(25,1);
    // bm_gpio_put(25, true);
    volatile uint32_t timer = TIMER_LENGTH;
    bm_gpio_put(25, false); // start low

    while(1) {
        
        bm_gpio_toggle(25);

        timer = TIMER_LENGTH;
        while ( timer >0) {
            timer = timer-1;
        }
        bm_gpio_toggle(25);

        timer = TIMER_LENGTH;
        while ( timer >0) {
            timer = timer-1;
        }
    }   
}