
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../SysTick/systick.h"

uint32_t clock_ms;

bool bm_systick_delay_ms(uint32_t ms) {
    clock_ms += ms;
    return true; 
}


bool bm_systick_get_ms(uint32_t *ms_out) {
    *ms_out = clock_ms;
    return true;
}

//real function returns true if timeout_ms has elapsed
bool bm_systick_timeout_elapsed(uint32_t start_ms, uint32_t timeout_ms) {
    return (clock_ms - start_ms) >= timeout_ms;
}