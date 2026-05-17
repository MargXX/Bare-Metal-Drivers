#include "gpio.h"

int main() {

    // bare metal programs never return from main
    // potential crash if we return from main, so just loop forever
    bm_gpio_enable(25);
    bm_gpio_set_direction(25,1);
    uint32_t timer = 0x00FFFFFF;

    while(1) {
        timer = 0x00FFFFFF;
        bm_gpio_toggle(25);

        while ( timer >0) {
            timer = timer-1;
        }
        bm_gpio_toggle(25);
    }   
}