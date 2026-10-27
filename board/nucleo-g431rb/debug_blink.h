/**
 * @file debug_blink.h
 * @brief Blinking LED functionality for debug purposes.
 *
 * This file contains declarations for functions that control the Nucleo
 * user LED (LD2 on PA5) to provide visual feedback during debugging and testing.
 */

#ifndef DEBUG_BLINK_H
#define DEBUG_BLINK_H

#include "gpio.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

// Nucleo-G431RB user LED LD2
#define DEBUG_LED_PIN BM_GPIO_PIN(BM_GPIO_PORT_A, 5)

//manipulates PA5 (LD2) to toggle every blink_rate_ms miliseconds in a loop
void blink_loop(uint32_t blink_rate_ms);

//manipulates PA5 (LD2) to blink a number of times, then returns
void blink_n_times(uint8_t blink_count);



#endif
