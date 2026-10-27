/**
 * @file hal_glue.c
 * @brief Points the ST HAL at this project's clock tree and SysTick timebase
 *
 * The HAL normally owns SysTick (HAL_InitTick / HAL_IncTick) and relies on
 * ST's system_stm32g4xx.c for the prescaler tables. Neither is used here:
 *   - bm_rcc_clock_init configures the clock tree and defines SystemCoreClock
 *   - platform/cortex-m/systick owns SysTick and its exception handler
 * so the HAL's weak timebase hooks are overridden to read the existing
 * millisecond count, and the prescaler tables are defined below.
 */

#include "stm32g4xx_hal.h"
#include "systick.h"
#include "rcc.h"
#include "hal_glue.h"

// prescaler shift tables declared in system_stm32g4xx.h, read by
// HAL_RCC_GetHCLKFreq / GetPCLK1Freq / GetPCLK2Freq. Values are the right
// shift for each RCC_CFGR.HPRE / PPREx encoding (RM0440 RCC_CFGR).
const uint8_t AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8]  = {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};

// SysTick is configured by bm_systick_init, not the HAL. Returning HAL_OK
// without touching SysTick keeps HAL_Init / HAL_RCC_ClockConfig from
// reprogramming the reload value if either is ever called.
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    (void)TickPriority;
    return HAL_OK;
}

// the SysTick ISR in systick.c increments the count; nothing to do here
void HAL_IncTick(void) {
}

// HAL timeouts compare against this. Backed by the bm_systick millisecond count.
uint32_t HAL_GetTick(void) {
    uint32_t ms = 0;
    (void)bm_systick_get_ms(&ms);
    return ms;
}

bool bm_hal_timebase_ready(void) {
    const uint32_t required = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
    return (SysTick->CTRL & required) == required;
}

bool bm_hal_clock_ready(void) {
    return (HAL_RCC_GetPCLK1Freq() == BM_RCC_SYSCLK_HZ) &&
           (HAL_RCC_GetPCLK2Freq() == BM_RCC_SYSCLK_HZ);
}
