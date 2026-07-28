
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../SysTick/systick.h"

bool bm_systick_delay_ms(uint32_t ms) {
    (void)ms;
    return true; 
}


bool bm_systick_get_ms(uint32_t *ms_out) {
    *ms_out = 0;
    return true;
}

//real function returns true if timeout_ms has elapsed
bool bm_systick_timeout_elapsed(uint32_t start_ms, uint32_t timeout_ms) {
    (void)start_ms;
    (void)timeout_ms;
    return true; 
}