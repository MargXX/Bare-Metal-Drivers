#include "systick.h"

static volatile uint32_t systick_ms_count = 0;

// initialize SysTick — must be called before any other SysTick functions
bool bm_systick_init(void) {

    //disable to prevent fires during setup
    SYST_CSR &= ~SYST_CSR_ENABLE_Msk; 
    //set to tick every 1 ms, or every  1000 microsecond
    SYST_RVR &= ~SYST_RVR_RELOAD_Msk;
    SYST_RVR |= SYSTICK_TICKS_PER_MS - 1;
    //clear countflag
    SYST_CVR = 0;

    systick_ms_count = 0; //reset count
    
    // set all bits
    //set clock source to processor clock
    //set to notify systick handler every millisecond automatically
    // turn back on
    SYST_CSR |= (SYST_CSR_CLKSOURCE_Msk | SYST_CSR_TICKINT_Msk | SYST_CSR_ENABLE_Msk);

    return true;
}

//resumes the SysTick counter
bool bm_systick_start(void) {
    SYST_CSR |= SYST_CSR_ENABLE_Msk;
    return true;
}

// stop the SysTick counter — tick count is preserved
bool bm_systick_stop(void) {
    SYST_CSR &= ~SYST_CSR_ENABLE_Msk;
    return true;
}

// reset the internal millisecond counter to 0 and clear SYST_CVR
bool bm_systick_reset(void) {
    //disable to prevent fires during operation
    SYST_CSR &= ~SYST_CSR_ENABLE_Msk; 
    //clear countflag
    SYST_CVR = 0;
    //reset count
    systick_ms_count = 0; 
    //enable to resume operation
    SYST_CSR |= SYST_CSR_ENABLE_Msk; 
    return true;
}

// blocking delay in milliseconds
bool bm_systick_delay_ms(uint32_t ms) {
    uint32_t start = systick_ms_count;
    while ((systick_ms_count - start) < ms) {}
    return true;
}

// returns elapsed milliseconds since init or last reset via ms_out
// rolls over after ~49.7 days
bool bm_systick_get_ms(uint32_t *ms_out) {
    if (ms_out == NULL) return false;
    *ms_out = systick_ms_count;
    return true;
}

// returns true if (current_ms - start_ms) >= timeout_ms
// returns false if not
// use bm_systick_get_ms() to capture start_ms
bool bm_systick_timeout_elapsed(uint32_t start_ms, uint32_t timeout_ms) {
    uint32_t current_ms;
    if (!bm_systick_get_ms(&current_ms)) {return false;}
    return (current_ms - start_ms) >= timeout_ms; 
}

// SysTick exception handler — must be defined in systick.c with this exact name
// increments the internal volatile tick counter each millisecond
void SysTick_Handler(void) {
    systick_ms_count++;
}