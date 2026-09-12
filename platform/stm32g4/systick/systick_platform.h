/**
 * @file systick_platform.h
 * @brief Public platform constants for stm32g431rb SysTick ive set in rcc_clock_init
 */


#ifndef SYSTICK_PLATFORM_H
#define SYSTICK_PLATFORM_H

#include "rcc.h"
#include "stm32g431xx.h"
#include <assert.h>


// ticks per millisecond — update if changing clock source
#define SYSTICK_TICKS_PER_MS (BM_RCC_SYSCLK_HZ / 1000UL)

// must be less than 2^24 or ~16.8 million (SysTick is 24-bit)
static_assert(SYSTICK_TICKS_PER_MS - 1UL <= SysTick_LOAD_RELOAD_Msk,
              "SysTick reload exceeds 24-bit RVR");


#endif /* SYSTICK_PLATFORM_H */