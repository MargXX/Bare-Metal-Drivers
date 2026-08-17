/**
 * @file systick.h
 * @brief SysTick driver API for the RP2040
 *
 * Provides millisecond timing using the ARM Cortex-M0+ SysTick timer.
 *
 * Clock assumption: external reference (1MHz watchdog tick, RP2040 default).
 *   1 tick = 1us, 1ms = 1000 ticks, RELOAD = 999
 *
 * To switch to processor clock: set SYST_CSR_CLKSOURCE_Msk in CSR and
 * update SYSTICK_TICKS_PER_MS to match your system clock frequency.
 *
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "systick_platform.h"



// returns false if error, true if functioned correctly

// initialize SysTick — must be called before any other SysTick functions
bool bm_systick_init(void);

// resumes the SysTick counter
bool bm_systick_start(void);

// stop the SysTick counter — tick count is preserved
bool bm_systick_stop(void);

// reset the internal millisecond counter to 0 and clear SYST_CVR
bool bm_systick_reset(void);

// blocking delay in milliseconds
bool bm_systick_delay_ms(uint32_t ms);

// returns elapsed milliseconds since init or last reset via ms_out
// rolls over after ~49.7 days
bool bm_systick_get_ms(uint32_t *ms_out);

// returns true if (current_ms - start_ms) >= timeout_ms
// returns false if not
// use bm_systick_get_ms() to capture start_ms
bool bm_systick_timeout_elapsed(uint32_t start_ms, uint32_t timeout_ms);

// SysTick exception handler — must be defined in systick.c with this exact name
// increments the internal volatile tick counter each millisecond
void isr_systick(void);

#endif /* SYSTICK_H */