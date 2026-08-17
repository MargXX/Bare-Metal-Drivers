/**
 * @file systick_platform.h
 * @brief Public platform constants for RP2040 SysTick
 */


#ifndef SYSTICK_PLATFORM_H
#define SYSTICK_PLATFORM_H

// ticks per millisecond — update if changing clock source
// 1MHz external reference (default): 1000
// 125MHz processor clock: 125000
#define SYSTICK_TICKS_PER_MS    125000U


#endif /* SYSTICK_PLATFORM_H */