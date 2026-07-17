/**
 * @file debug_blink.h
 * @brief Blinking LED functionality for debug purposes.
 *
 * This file contains declarations for functions that control an onboard LED
 * to provide visual feedback during debugging and testing.
 */




#ifndef DEBUG_BLINK_H
#define DEBUG_BLINK_H

#include <stdint.h>
#include <stdbool.h>

//manipulates GPIO 25 (onboard LED) to toggle every blink_rate_ms miliseconds in a loop
void blink_loop(uint8_t blink_rate_ms);

//manipulates GPIO 25 (onboard LED) to blink a number of times, then returns
void blink_n_times(uint8_t blink_count);



#endif