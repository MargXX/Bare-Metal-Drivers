/**
 * @file gpio.h
 * @brief GPIO driver API for the RP2040
 *
 * Provides pin function selection, direction control, and
 * read/write operations for the RP2040 GPIO peripheral.
 *
 * Reference: RP2040 Datasheet, Sections 2.3 (SIO), 2.19 (GPIO)
 */

#ifndef GPIO_H
#define GPIO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gpio_reg.h"



// returns false if error, true if functioned correctly

// set the function of the GPIO pin
bool bm_gpio_set_function(uint8_t pin, uint8_t function);

// Set pin direction: 1 = output, 0 = input
bool bm_gpio_set_direction(uint8_t pin, uint8_t direction);

// Set output value high or low
bool bm_gpio_put(uint8_t pin, bool value);

// Read input value (returns 0 or 1)
bool bm_gpio_get(uint8_t pin, bool *value);



//enables GPIO on input with SIO
bool bm_gpio_enable(uint8_t pin) ;

//force pin high
bool bm_gpio_set(uint8_t pin);

//forces a pin low
bool bm_gpio_clear(uint8_t pin);

//flips pin
bool bm_gpio_toggle(uint8_t pin);

 
#endif /* GPIO_H */